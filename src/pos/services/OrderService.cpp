
#include "pos/services/OrderService.hpp"
#include "pos/common/PosError.hpp"

using namespace std;

// removes extra spaces, "  Ali   Raza " becomes "Ali Raza"
static string normalizeSpaces(const string& text) {
    string result;
    bool spaceWaiting = false;
    int length = static_cast<int>(text.size());
    for (int i = 0; i < length; i++) {
        if (text[i] == ' ') {
            if (!result.empty()) {
                spaceWaiting = true;
            }
        }
        else {
            if (spaceWaiting) {
                result += ' ';
                spaceWaiting = false;
            }
            result += text[i];
        }
    }
    return result;
}

// position of the line with this barcode, -1 if it is not in the order
static int findLineIndex(const Order& order, const string& barcode) {
    int count = static_cast<int>(order.lines.size());
    for (int i = 0; i < count; i++) {
        if (order.lines[i].barcode == barcode) {
            return i;
        }
    }
    return -1;
}

OrderService::OrderService(IProductRepository& products,
    ProductService& service,
    IOrderRepository& orders,
    IOrderDetailRepository& orderDetails,
    IdGenerator& ids,
    const IClock& clockToUse)
    : productRepository(products),
    productService(service),
    orderRepository(orders),
    orderDetailRepository(orderDetails),
    idGenerator(ids),
    clock(clockToUse),
    hasDraft(false),
    customerIsSet(false),
    paymentIsSet(false) {
}

// all the other functions need an order that is started
void OrderService::requireDraft() const {
    if (!hasDraft) {
        throw PosError("No order is started");
    }
}

void OrderService::start() {
    Order order;
    order.id = "";                       // id is given when the order is saved
    order.date = "";
    order.time = "";
    order.customerName = "";
    order.contact = "-";
    order.status = OrderStatus::InProgress;
    order.payment = Money();
    order.mode = PaymentMode::Cash;
    order.merchant = "-";

    draftOrder = order;
    hasDraft = true;
    customerIsSet = false;
    paymentIsSet = false;
}

void OrderService::addItem(const string& barcode, double quantity) {
    requireDraft();

    if (!(quantity > 0)) {
        throw PosError("Quantity must be more than zero");
    }

    optional<Product> found = productRepository.findByBarcode(barcode);
    if (!found.has_value()) {
        throw PosError("Product not found for barcode: " + barcode);
    }
    Product product = found.value();

    // same product again, so add to the quantity that is already there
    int lineIndex = findLineIndex(draftOrder, barcode);
    double quantityAlready = 0;
    if (lineIndex >= 0) {
        quantityAlready = draftOrder.lines[lineIndex].quantity;
    }
    double totalQuantity = quantityAlready + quantity;

    // stock must be enough for everything in the order, not only this scan
    if (!productService.canSell(barcode, totalQuantity)) {
        throw PosError("Cannot add " + quantityToText(quantity) + " of " + product.acronym +
            " (available: " + quantityToText(product.availableQty) +
            ", already in this order: " + quantityToText(quantityAlready) + ")");
    }

    if (lineIndex >= 0) {
        draftOrder.lines[lineIndex].quantity = totalQuantity;
    }
    else {
        // copy the price, tax and discount now, because the product
        // price can change later and this order should not change
        OrderLine line = { product.id, product.barcode, product.acronym,
                          product.unitPrice, product.tax, product.discount, quantity };
        draftOrder.lines.insert(draftOrder.lines.end(), 1, line);
    }
}

void OrderService::removeItem(const string& barcode) {
    requireDraft();

    int lineIndex = findLineIndex(draftOrder, barcode);
    if (lineIndex < 0) {
        throw PosError("This product is not in the order: " + barcode);
    }
    draftOrder.lines.erase(draftOrder.lines.begin() + lineIndex);
}

void OrderService::updateItemQuantity(const string& barcode, double newQuantity) {
    requireDraft();

    if (!(newQuantity > 0)) {
        throw PosError("Quantity must be more than zero");
    }

    int lineIndex = findLineIndex(draftOrder, barcode);
    if (lineIndex < 0) {
        throw PosError("This product is not in the order: " + barcode);
    }

    optional<Product> found = productRepository.findByBarcode(barcode);
    if (!found.has_value()) {
        throw PosError("Product not found for barcode: " + barcode);
    }

    if (!productService.canSell(barcode, newQuantity)) {
        throw PosError("Cannot set quantity to " + quantityToText(newQuantity) +
            " (available stock: " + quantityToText(found->availableQty) + ")");
    }

    draftOrder.lines[lineIndex].quantity = newQuantity;
}

void OrderService::holdCurrentOrder() {
    requireDraft();
    if (draftOrder.lines.empty()) {
        throw PosError("Cannot hold an empty order");
    }

    draftOrder.status = OrderStatus::OnHold;
    heldOrders.push_back(draftOrder);

    start();
}

bool OrderService::hasHeldOrders() const {
    return !heldOrders.empty();
}

vector<Order> OrderService::getHeldOrders() const {
    return heldOrders;
}

void OrderService::resumeHeldOrder(size_t index) {
    if (heldOrders.empty()) {
        throw PosError("No orders are on hold");
    }
    if (index >= heldOrders.size()) {
        throw PosError("Invalid held order index");
    }

    draftOrder = heldOrders[index];
    draftOrder.status = OrderStatus::InProgress;
    hasDraft = true;
    customerIsSet = !draftOrder.customerName.empty();
    paymentIsSet = false;

    heldOrders.erase(heldOrders.begin() + index);
}


void OrderService::setCustomer(const string& name, const string& contact) {
    requireDraft();

    // the saved line is read back by splitting on spaces,
    // so the name cannot be empty and the contact cannot have spaces
    string cleanName = normalizeSpaces(name);
    if (cleanName.empty()) {
        throw PosError("Customer name is required");
    }
    if (cleanName.find('#') != string::npos) {
        throw PosError("Customer name cannot contain #");
    }

    string cleanContact = normalizeSpaces(contact);
    if (cleanContact.empty()) {
        cleanContact = "-";
    }
    if (cleanContact.find(' ') != string::npos || cleanContact.find('#') != string::npos) {
        throw PosError("Contact cannot contain spaces or #");
    }

    draftOrder.customerName = cleanName;
    draftOrder.contact = cleanContact;
    customerIsSet = true;
}

void OrderService::setPayment(PaymentMode mode, const string& merchant) {
    requireDraft();

    string cleanMerchant = normalizeSpaces(merchant);
    if (cleanMerchant.empty()) {
        cleanMerchant = "-";
    }
    if (cleanMerchant.find(' ') != string::npos || cleanMerchant.find('#') != string::npos) {
        throw PosError("Merchant cannot contain spaces or #");
    }

    draftOrder.mode = mode;
    draftOrder.merchant = cleanMerchant;
    paymentIsSet = true;
}

const Order& OrderService::draft() const {
    requireDraft();
    return draftOrder;
}

Order OrderService::finalize() {
    requireDraft();

    if (draftOrder.lines.empty()) {
        throw PosError("The order has no items");
    }
    if (!customerIsSet) {
        throw PosError("Customer name is not set");
    }
    if (!paymentIsSet) {
        throw PosError("Payment mode is not set");
    }

    int lineCount = static_cast<int>(draftOrder.lines.size());

    // stock may have changed since the items were added, so check again
    for (int i = 0; i < lineCount; i++) {
        const OrderLine& line = draftOrder.lines[i];
        if (!productService.canSell(line.barcode, line.quantity)) {
            throw PosError("Not enough stock any more for " + line.acronym);
        }
    }

    // total to pay, line by line: price x quantity + tax x quantity - discount x quantity
    Money payable;
    for (int i = 0; i < lineCount; i++) {
        const OrderLine& line = draftOrder.lines[i];
        Money lineAmount = line.unitPrice * line.quantity;
        Money lineTax = line.tax * line.quantity;
        Money lineDiscount = line.discount * line.quantity;
        payable = payable + lineAmount + lineTax - lineDiscount;
    }

    Order order = draftOrder;
    order.id = idGenerator.nextOrderId();
    order.date = clock.today();
    order.time = clock.now();
    order.status = OrderStatus::Paid;
    order.payment = payable;

    // save the order, then its lines, then take the goods out of stock
    orderRepository.add(order);
    for (int i = 0; i < lineCount; i++) {
        orderDetailRepository.add(order.id, order.lines[i]);
    }
    for (int i = 0; i < lineCount; i++) {
        productService.reduceStock(order.lines[i].barcode, order.lines[i].quantity);
    }

    // this order is finished, a new one needs start() again
    hasDraft = false;
    customerIsSet = false;
    paymentIsSet = false;
    return order;
}
