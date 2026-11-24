// OrderService.cpp                                    Owner: Person 5
// Implementation of OrderService for draft management, hold/resume, and order editing.

#include "pos/services/OrderService.hpp"

#include <algorithm>
#include <map>

#include "pos/common/PosError.hpp"
#include "pos/domain/Product.hpp"
#include "pos/services/EditPolicy.hpp"

using namespace std;

static int findLineIndex(const Order& order, const string& barcode) {
    for (size_t i = 0; i < order.lines.size(); ++i) {
        if (order.lines[i].barcode == barcode) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

OrderService::OrderService(IProductRepository& products,
                           ProductService& productService,
                           IOrderRepository& orders,
                           IOrderDetailRepository& orderDetails,
                           IdGenerator& idGenerator,
                           const IClock& clock)
    : productRepository(products),
      productService(productService),
      orderRepository(orders),
      orderDetailRepository(orderDetails),
      idGenerator(idGenerator),
      clock(clock) {
}

void OrderService::requireDraft() const {
    if (!hasDraft) {
        throw PosError("No active order. Call start() first.");
    }
}

void OrderService::start() {
    Order order;
    order.id = "";
    order.date = clock.today();
    order.time = clock.now();
    order.customerName = "Walk-in";
    order.contact = "-";
    order.status = OrderStatus::InProgress;
    order.payment = Money();
    order.mode = PaymentMode::Cash;
    order.merchant = "-";

    draftOrder = order;
    hasDraft = true;
    customerIsSet = false;
    paymentIsSet = false;
    isEditingExisting_ = false;
    originalLines_.clear();
}

void OrderService::addItem(const string& barcode, double quantity) {
    requireDraft();

    if (!(quantity > 0.0)) {
        throw PosError("Quantity must be more than zero");
    }

    optional<Product> found = productRepository.findByBarcode(barcode);
    if (!found.has_value()) {
        throw PosError("Product not found for barcode: " + barcode);
    }
    Product product = found.value();

    int lineIndex = findLineIndex(draftOrder, barcode);
    double quantityAlready = (lineIndex >= 0) ? draftOrder.lines[lineIndex].quantity : 0.0;
    double totalQuantity = quantityAlready + quantity;

    if (!productService.canSell(barcode, totalQuantity)) {
        throw PosError("Cannot add " + quantityToText(quantity) + " of " + product.acronym +
                       " (available: " + quantityToText(product.availableQty) +
                       ", already in this order: " + quantityToText(quantityAlready) + ")");
    }

    if (lineIndex >= 0) {
        draftOrder.lines[lineIndex].quantity = totalQuantity;
    } else {
        OrderLine line{product.id, product.barcode, product.acronym,
                       product.unitPrice, product.tax, product.discount, quantity};
        draftOrder.lines.push_back(line);
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

    if (!(newQuantity > 0.0)) {
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
    Product product = found.value();

    if (newQuantity > draftOrder.lines[lineIndex].quantity) {
        double diff = newQuantity - draftOrder.lines[lineIndex].quantity;
        if (!productService.canSell(barcode, diff)) {
            throw PosError("Not enough stock for " + product.acronym +
                           " (available: " + quantityToText(product.availableQty) + ")");
        }
    }

    draftOrder.lines[lineIndex].quantity = newQuantity;
}

void OrderService::hold() {
    requireDraft();
    if (draftOrder.lines.empty()) {
        throw PosError("Cannot hold an empty order");
    }

    if (draftOrder.id.empty()) {
        draftOrder.id = idGenerator.nextOrderId();
    }
    draftOrder.status = OrderStatus::OnHold;
    draftOrder.date = clock.today();
    draftOrder.time = clock.now();

    Money totalPayment;
    for (const auto& line : draftOrder.lines) {
        totalPayment += (line.unitPrice * line.quantity) + (line.tax * line.quantity) - (line.discount * line.quantity);
    }
    draftOrder.payment = totalPayment;

    // Persist to storage
    optional<Order> existing = orderRepository.findById(draftOrder.id);
    if (existing.has_value()) {
        orderRepository.update(draftOrder);
    } else {
        orderRepository.add(draftOrder);
    }
    orderDetailRepository.replaceForOrder(draftOrder.id, draftOrder.lines);

    memoryHeldOrders_.push_back(draftOrder);
    start();
}

vector<Order> OrderService::heldOrders() const {
    vector<Order> allOrders = orderRepository.all();
    vector<Order> held;
    for (auto ord : allOrders) {
        if (ord.status == OrderStatus::OnHold) {
            // Attach lines
            vector<OrderLine> lines = orderDetailRepository.findByOrderId(ord.id);
            for (auto& l : lines) {
                optional<Product> p = productRepository.findByBarcode(l.barcode);
                if (p.has_value()) {
                    l.acronym = p->acronym;
                    l.unitPrice = p->unitPrice;
                    l.tax = p->tax;
                    l.discount = p->discount;
                    l.productId = p->id;
                }
            }
            ord.lines = lines;
            held.push_back(ord);
        }
    }
    if (held.empty()) {
        return memoryHeldOrders_;
    }
    return held;
}

void OrderService::resumeLatest() {
    vector<Order> held = heldOrders();
    if (held.empty()) {
        throw PosError("No orders are on hold");
    }
    resume(held.back().id);
}

void OrderService::resume(const string& orderId) {
    optional<Order> found = orderRepository.findById(orderId);
    if (!found.has_value()) {
        // Look in memory
        for (const auto& o : memoryHeldOrders_) {
            if (o.id == orderId) {
                found = o;
                break;
            }
        }
    }

    if (!found.has_value()) {
        throw PosError("Held order not found: " + orderId);
    }

    Order ord = found.value();
    vector<OrderLine> lines = orderDetailRepository.findByOrderId(ord.id);
    for (auto& l : lines) {
        optional<Product> p = productRepository.findByBarcode(l.barcode);
        if (p.has_value()) {
            l.acronym = p->acronym;
            l.unitPrice = p->unitPrice;
            l.tax = p->tax;
            l.discount = p->discount;
            l.productId = p->id;
        }
    }
    ord.lines = lines;
    ord.status = OrderStatus::InProgress;
    orderRepository.update(ord);

    draftOrder = ord;
    hasDraft = true;
    customerIsSet = !draftOrder.customerName.empty() && draftOrder.customerName != "Walk-in";
    paymentIsSet = true;
    isEditingExisting_ = false;

    // Remove from memory held list
    for (auto it = memoryHeldOrders_.begin(); it != memoryHeldOrders_.end(); ++it) {
        if (it->id == orderId) {
            memoryHeldOrders_.erase(it);
            break;
        }
    }
}

void OrderService::resumeHeldOrder(size_t index) {
    vector<Order> held = heldOrders();
    if (held.empty()) {
        throw PosError("No orders are on hold");
    }
    if (index >= held.size()) {
        throw PosError("Invalid held order index");
    }
    resume(held[index].id);
}

void OrderService::loadOrderForEdit(const string& orderId, bool managerApproved) {
    optional<Order> found = orderRepository.findById(orderId);
    if (!found.has_value()) {
        throw PosError("Order not found for edit: " + orderId);
    }
    Order ord = found.value();

    // Enforce 7-day manager approval rule
    EditPolicy::check(ord, clock.today(), managerApproved);

    vector<OrderLine> lines = orderDetailRepository.findByOrderId(ord.id);
    for (auto& l : lines) {
        optional<Product> p = productRepository.findByBarcode(l.barcode);
        if (p.has_value()) {
            l.acronym = p->acronym;
            l.unitPrice = p->unitPrice;
            l.tax = p->tax;
            l.discount = p->discount;
            l.productId = p->id;
        }
    }
    ord.lines = lines;

    draftOrder = ord;
    hasDraft = true;
    customerIsSet = true;
    paymentIsSet = true;
    isEditingExisting_ = true;
    originalLines_ = ord.lines;
}

void OrderService::setCustomer(const string& name, const string& contact) {
    requireDraft();
    if (name.empty()) {
        throw PosError("Customer name is required");
    }
    draftOrder.customerName = name;
    draftOrder.contact = contact.empty() ? "-" : contact;
    customerIsSet = true;
}

void OrderService::setPayment(PaymentMode mode, const string& merchant) {
    requireDraft();
    draftOrder.mode = mode;
    draftOrder.merchant = merchant.empty() ? "-" : merchant;
    paymentIsSet = true;
}

const Order& OrderService::draft() const {
    requireDraft();
    return draftOrder;
}

Order OrderService::finalize() {
    requireDraft();

    if (draftOrder.lines.empty()) {
        throw PosError("Cannot finalize an order with no items");
    }
    if (!customerIsSet && draftOrder.customerName.empty()) {
        throw PosError("Customer details must be set before finalizing");
    }
    if (!paymentIsSet) {
        throw PosError("Payment mode must be set before finalizing");
    }

    // Verify stock availability
    for (const auto& line : draftOrder.lines) {
        if (!productService.canSell(line.barcode, line.quantity)) {
            throw PosError("Not enough stock for " + line.acronym);
        }
    }

    // Reduce stock
    for (const auto& line : draftOrder.lines) {
        productService.reduceStock(line.barcode, line.quantity);
    }

    Money totalPayment;
    for (const auto& line : draftOrder.lines) {
        totalPayment += (line.unitPrice * line.quantity) + (line.tax * line.quantity) - (line.discount * line.quantity);
    }
    draftOrder.payment = totalPayment;

    if (draftOrder.id.empty()) {
        draftOrder.id = idGenerator.nextOrderId();
    }
    draftOrder.date = clock.today();
    draftOrder.time = clock.now();
    draftOrder.status = OrderStatus::Paid;

    // Update or append order
    optional<Order> existing = orderRepository.findById(draftOrder.id);
    if (existing.has_value()) {
        orderRepository.update(draftOrder);
    } else {
        orderRepository.add(draftOrder);
    }

    orderDetailRepository.replaceForOrder(draftOrder.id, draftOrder.lines);

    Order finished = draftOrder;
    hasDraft = false;
    isEditingExisting_ = false;
    originalLines_.clear();
    return finished;
}

Order OrderService::finalizeEdit() {
    requireDraft();
    if (!isEditingExisting_) {
        return finalize();
    }

    if (draftOrder.lines.empty()) {
        throw PosError("Cannot save an edited order with zero items");
    }

    // Map original quantities
    map<string, double> oldQtys;
    for (const auto& l : originalLines_) {
        oldQtys[l.barcode] = l.quantity;
    }

    map<string, double> newQtys;
    for (const auto& l : draftOrder.lines) {
        newQtys[l.barcode] = l.quantity;
    }

    // Apply stock differences
    for (const auto& [barcode, newQ] : newQtys) {
        double oldQ = oldQtys.count(barcode) ? oldQtys[barcode] : 0.0;
        double diff = newQ - oldQ;
        if (diff > 0.0) {
            // Need more stock
            if (!productService.canSell(barcode, diff)) {
                throw PosError("Not enough stock to increase quantity for " + barcode);
            }
            productService.reduceStock(barcode, diff);
        } else if (diff < 0.0) {
            // Return stock
            optional<Product> p = productRepository.findByBarcode(barcode);
            if (p.has_value()) {
                productRepository.updateQuantity(p->id, p->availableQty + (-diff));
            }
        }
    }

    // Products that were completely removed: return full old quantity
    for (const auto& [barcode, oldQ] : oldQtys) {
        if (!newQtys.count(barcode)) {
            optional<Product> p = productRepository.findByBarcode(barcode);
            if (p.has_value()) {
                productRepository.updateQuantity(p->id, p->availableQty + oldQ);
            }
        }
    }

    Money totalPayment;
    for (const auto& line : draftOrder.lines) {
        totalPayment += (line.unitPrice * line.quantity) + (line.tax * line.quantity) - (line.discount * line.quantity);
    }
    draftOrder.payment = totalPayment;

    draftOrder.date = clock.today();
    draftOrder.time = clock.now();
    draftOrder.status = OrderStatus::Paid;

    orderRepository.update(draftOrder);
    orderDetailRepository.replaceForOrder(draftOrder.id, draftOrder.lines);

    Order finished = draftOrder;
    hasDraft = false;
    isEditingExisting_ = false;
    originalLines_.clear();
    return finished;
}
