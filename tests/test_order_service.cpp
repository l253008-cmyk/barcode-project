#include <cassert>
#include <iostream>
#include "pos/common/IClock.hpp"
#include "pos/common/Money.hpp"
#include "pos/common/PosError.hpp"
#include "pos/domain/Bill.hpp"
#include "pos/domain/Order.hpp"
#include "pos/services/IdGenerator.hpp"
#include "pos/services/OrderService.hpp"
#include "pos/services/ProductService.hpp"
#include "pos/services/StockPolicy.hpp"
#include "pos/storage/IBillRepository.hpp"
#include "pos/storage/IOrderDetailRepository.hpp"
#include "pos/storage/IOrderRepository.hpp"
#include "pos/storage/IProductRepository.hpp"
using namespace std;

// ---------- fake repositories, they keep everything in lists so no file is needed ----------

class FakeProductRepository : public IProductRepository {
public:
    vector<Product> products;

    vector<Product> all() const override {
        return products;
    }

    optional<Product> findByBarcode(const string& barcode) const override {
        int count = static_cast<int>(products.size());
        for (int i = 0; i < count; i++) {
            if (products[i].barcode == barcode) {
                return products[i];
            }
        }
        return nullopt;
    }

    optional<Product> findById(int id) const override {
        int count = static_cast<int>(products.size());
        for (int i = 0; i < count; i++) {
            if (products[i].id == id) {
                return products[i];
            }
        }
        return nullopt;
    }

    void updateQuantity(int id, double newQuantity) override {
        int count = static_cast<int>(products.size());
        for (int i = 0; i < count; i++) {
            if (products[i].id == id) {
                products[i].availableQty = newQuantity;
            }
        }
    }

    void add(const Product& product) override {
        products.push_back(product);
    }
};

class FakeOrderRepository : public IOrderRepository {
public:
    vector<Order> orders;

    void add(const Order& order) override {
        orders.insert(orders.end(), 1, order);
    }

    string lastId() const override {
        if (orders.empty()) {
            return "";
        }
        return orders.back().id;
    }
};

// one saved order line together with its order id
struct DetailRow {
    string orderId;
    OrderLine line;
};

class FakeOrderDetailRepository : public IOrderDetailRepository {
public:
    vector<DetailRow> rows;

    void add(const string& orderId, const OrderLine& line) override {
        DetailRow row = { orderId, line };
        rows.insert(rows.end(), 1, row);
    }
};

class FakeBillRepository : public IBillRepository {
public:
    void add(const Bill&) override {
    }

    string lastId() const override {
        return "";
    }
};

// clock that always says 14 November 2026, 10:32:05
class FakeClock : public IClock {
public:
    string today() const override {
        return "2026-11-14";
    }

    string now() const override {
        return "10:32:05";
    }

    long long millis() const override {
        return 0;
    }
};

// ---------- the three products from the proposal ----------

static Product makeTea() {
    Product tea = { 1, "100001", "TEA", "Tetley Tea Bags 25", MeasurementUnit::Count,
                   Money::parse("350.00"), Money::parse("17.50"), Money::parse("10.00"),
                   40, "data/barcodes/100001.bmp" };
    return tea;
}

static Product makeSugar() {
    Product sugar = { 2, "100002", "SUGAR", "Sugar Loose", MeasurementUnit::Weight,
                     Money::parse("180.00"), Money::parse("0.00"), Money::parse("0.00"),
                     25.5, "data/barcodes/100002.bmp" };
    return sugar;
}

static Product makeBiscuit() {
    Product biscuit = { 3, "100003", "BISCUIT", "Sooper Biscuit", MeasurementUnit::Count,
                       Money::parse("60.00"), Money::parse("3.00"), Money::parse("5.00"),
                       100, "data/barcodes/100003.bmp" };
    return biscuit;
}

// a shop with the three products and 122 old orders (the last id is 00012214),
// so the next order gets 00012314 like in the proposal
// every test makes its own new Shop, so tests do not affect each other
class Shop {
public:
    FakeProductRepository products;
    StockPolicy policy;
    FakeOrderRepository orders;
    FakeOrderDetailRepository details;
    FakeBillRepository bills;
    FakeClock fakeClock;
    ProductService productService;
    IdGenerator idGenerator;
    OrderService orderService;

    Shop()
        : productService(products, policy),
        idGenerator(orders, bills, fakeClock),   // change this line if IdGenerator needs other arguments
        orderService(products, productService, orders, details, idGenerator, fakeClock) {
        products.products.insert(products.products.end(), 1, makeTea());
        products.products.insert(products.products.end(), 1, makeSugar());
        products.products.insert(products.products.end(), 1, makeBiscuit());

        Order earlier;
        earlier.id = "00012214";
        earlier.date = "2026-11-14";
        earlier.time = "09:00:00";
        earlier.customerName = "Earlier Customer";
        earlier.contact = "-";
        earlier.status = OrderStatus::Paid;
        earlier.payment = Money();
        earlier.mode = PaymentMode::Cash;
        earlier.merchant = "-";
        orders.orders.insert(orders.orders.end(), 1, earlier);
    }
};

// ---------- helpers, they give true when the call throws a PosError ----------

static bool addItemThrows(OrderService& service, const string& barcode, double quantity) {
    try {
        service.addItem(barcode, quantity);
    }
    catch (const PosError&) {
        return true;
    }
    return false;
}

static bool removeItemThrows(OrderService& service, const string& barcode) {
    try {
        service.removeItem(barcode);
    }
    catch (const PosError&) {
        return true;
    }
    return false;
}

static bool setCustomerThrows(OrderService& service, const string& name, const string& contact) {
    try {
        service.setCustomer(name, contact);
    }
    catch (const PosError&) {
        return true;
    }
    return false;
}

static bool setPaymentThrows(OrderService& service, PaymentMode mode, const string& merchant) {
    try {
        service.setPayment(mode, merchant);
    }
    catch (const PosError&) {
        return true;
    }
    return false;
}

static bool finalizeThrows(OrderService& service) {
    try {
        service.finalize();
    }
    catch (const PosError&) {
        return true;
    }
    return false;
}

static bool draftThrows(const OrderService& service) {
    try {
        service.draft();
    }
    catch (const PosError&) {
        return true;
    }
    return false;
}

int main() {
    // ---------- the order from the proposal ----------
    {
        Shop shop;
        shop.orderService.start();
        shop.orderService.setCustomer("Ali Raza", "0300-1234567");
        shop.orderService.addItem("100001", 2);
        shop.orderService.addItem("100002", 1.5);
        shop.orderService.setPayment(PaymentMode::Cash, "");
        assert(shop.orderService.draft().lines.size() == 2);

        Order saved = shop.orderService.finalize();

        assert(saved.id == "00012314");
        assert(saved.date == "2026-11-14");
        assert(saved.time == "10:32:05");
        assert(saved.customerName == "Ali Raza");
        assert(saved.contact == "0300-1234567");
        assert(saved.status == OrderStatus::Paid);
        assert(saved.payment.toString() == "985.00");
        assert(saved.mode == PaymentMode::Cash);
        assert(saved.merchant == "-");
        assert(saved.lines.size() == 2);
        assert(saved.lines[0].unitPrice.toString() == "350.00");   // price copied at the time of sale
        assert(saved.lines[0].tax.toString() == "17.50");

        // the order is saved
        assert(shop.orders.orders.size() == 2);
        assert(shop.orders.orders[1].id == "00012314");

        // its lines are saved
        assert(shop.details.rows.size() == 2);
        assert(shop.details.rows[0].orderId == "00012314");
        assert(shop.details.rows[0].line.productId == 1);
        assert(shop.details.rows[0].line.barcode == "100001");
        assert(shop.details.rows[0].line.quantity == 2);
        assert(shop.details.rows[1].orderId == "00012314");
        assert(shop.details.rows[1].line.productId == 2);
        assert(shop.details.rows[1].line.quantity == 1.5);

        // stock is reduced
        assert(shop.products.findById(1).value().availableQty == 38);
        assert(shop.products.findById(2).value().availableQty == 24);
        assert(shop.products.findById(3).value().availableQty == 100);

        // the order is finished, so a new one needs start()
        assert(draftThrows(shop.orderService));
        assert(finalizeThrows(shop.orderService));

        // next order gets the next id
        shop.orderService.start();
        shop.orderService.setCustomer("Sara Khan", "");
        shop.orderService.setPayment(PaymentMode::Wallet, "Easypaisa");
        shop.orderService.addItem("100003", 1);
        Order second = shop.orderService.finalize();
        assert(second.id == "00012414");
    }

    // ---------- tax and discount: BISCUIT x 3 = 3 x (60.00 + 3.00 - 5.00) ----------
    {
        Shop shop;
        shop.orderService.start();
        shop.orderService.setCustomer("Sara Khan", "-");
        shop.orderService.setPayment(PaymentMode::Card, "HBL");
        shop.orderService.addItem("100003", 3);
        Order saved = shop.orderService.finalize();
        assert(saved.payment.toString() == "174.00");
        assert(saved.mode == PaymentMode::Card);
        assert(saved.merchant == "HBL");
        assert(saved.contact == "-");
    }

    // ---------- stock rule: TEA x 41 is rejected, TEA x 40 is accepted ----------
    {
        Shop shop;
        shop.orderService.start();
        assert(addItemThrows(shop.orderService, "100001", 41));
        assert(shop.orderService.draft().lines.empty());          // nothing was added
        shop.orderService.addItem("100001", 40);
        assert(shop.orderService.draft().lines.size() == 1);
        assert(shop.orderService.draft().lines[0].quantity == 40);
    }

    // ---------- wrong items ----------
    {
        Shop shop;
        shop.orderService.start();
        assert(addItemThrows(shop.orderService, "999999", 1));    // not in Product.txt
        assert(addItemThrows(shop.orderService, "", 1));
        assert(addItemThrows(shop.orderService, "100001", 0));
        assert(addItemThrows(shop.orderService, "100001", -2));
        assert(addItemThrows(shop.orderService, "100001", 1.5));  // count product, not a whole number
        shop.orderService.addItem("100002", 25.5);                // weight product, decimals are fine
        assert(addItemThrows(shop.orderService, "100002", 0.1));  // now it is more than the stock
        assert(shop.orderService.draft().lines.size() == 1);
    }

    // ---------- same product scanned again ----------
    {
        Shop shop;
        shop.orderService.start();
        shop.orderService.addItem("100001", 20);
        shop.orderService.addItem("100001", 20);
        assert(shop.orderService.draft().lines.size() == 1);      // one line, not two
        assert(shop.orderService.draft().lines[0].quantity == 40);
        assert(addItemThrows(shop.orderService, "100001", 1));    // 41 in total is too much
        assert(shop.orderService.draft().lines[0].quantity == 40);
    }

    // ---------- removing an item ----------
    {
        Shop shop;
        shop.orderService.start();
        shop.orderService.addItem("100001", 2);
        shop.orderService.addItem("100002", 1);
        shop.orderService.removeItem("100001");
        assert(shop.orderService.draft().lines.size() == 1);
        assert(shop.orderService.draft().lines[0].barcode == "100002");
        assert(removeItemThrows(shop.orderService, "100001"));    // already removed
        assert(removeItemThrows(shop.orderService, "999999"));
    }

    // ---------- customer and payment ----------
    {
        Shop shop;
        shop.orderService.start();
        assert(setCustomerThrows(shop.orderService, "", ""));
        assert(setCustomerThrows(shop.orderService, "   ", ""));
        assert(setCustomerThrows(shop.orderService, "Ali#Raza", ""));
        assert(setCustomerThrows(shop.orderService, "Ali Raza", "0300 1234567"));   // space in contact
        assert(setCustomerThrows(shop.orderService, "Ali Raza", "03#00"));

        shop.orderService.setCustomer("  Ali   Raza ", "");
        assert(shop.orderService.draft().customerName == "Ali Raza");   // extra spaces removed
        assert(shop.orderService.draft().contact == "-");               // empty contact becomes -

        assert(setPaymentThrows(shop.orderService, PaymentMode::Card, "Bank Alfalah"));
        assert(setPaymentThrows(shop.orderService, PaymentMode::Card, "HB#L"));
        shop.orderService.setPayment(PaymentMode::Cash, "");
        assert(shop.orderService.draft().merchant == "-");
    }

    // ---------- finalize needs everything ----------
    {
        Shop shop;
        shop.orderService.start();
        assert(finalizeThrows(shop.orderService));                // no items
        shop.orderService.addItem("100001", 1);
        assert(finalizeThrows(shop.orderService));                // no customer
        shop.orderService.setCustomer("Ali Raza", "");
        assert(finalizeThrows(shop.orderService));                // no payment mode
        shop.orderService.setPayment(PaymentMode::Cash, "");

        assert(shop.orders.orders.size() == 1);                   // nothing saved so far
        assert(shop.details.rows.empty());
        assert(shop.products.findById(1).value().availableQty == 40);

        shop.orderService.finalize();                             // now it works
        assert(shop.orders.orders.size() == 2);
    }

    // ---------- stock is checked again when the order is saved ----------
    {
        Shop shop;
        shop.orderService.start();
        shop.orderService.setCustomer("Ali Raza", "");
        shop.orderService.setPayment(PaymentMode::Cash, "");
        shop.orderService.addItem("100001", 40);
        shop.productService.reduceStock("100001", 5);             // another sale took 5 TEA
        assert(finalizeThrows(shop.orderService));
        assert(shop.orders.orders.size() == 1);                   // nothing saved
        assert(shop.details.rows.empty());
        assert(shop.products.findById(1).value().availableQty == 35);
        assert(shop.orderService.draft().lines.size() == 1);      // draft is kept
    }

    // ---------- nothing works before start() ----------
    {
        Shop shop;
        assert(addItemThrows(shop.orderService, "100001", 1));
        assert(removeItemThrows(shop.orderService, "100001"));
        assert(setCustomerThrows(shop.orderService, "Ali Raza", ""));
        assert(setPaymentThrows(shop.orderService, PaymentMode::Cash, ""));
        assert(finalizeThrows(shop.orderService));
        assert(draftThrows(shop.orderService));
    }

    // ---------- start() throws away an old draft ----------
    {
        Shop shop;
        shop.orderService.start();
        shop.orderService.addItem("100001", 2);
        shop.orderService.start();
        assert(shop.orderService.draft().lines.empty());
    }

    // ---------- updateItemQuantity ----------
    {
        Shop shop;
        shop.orderService.start();
        shop.orderService.addItem("100001", 2);
        assert(shop.orderService.draft().lines[0].quantity == 2);

        // increase quantity
        shop.orderService.updateItemQuantity("100001", 5);
        assert(shop.orderService.draft().lines[0].quantity == 5);

        // decrease quantity
        shop.orderService.updateItemQuantity("100001", 1);
        assert(shop.orderService.draft().lines[0].quantity == 1);

        // set to <= 0 throws PosError
        bool threwZero = false;
        try {
            shop.orderService.updateItemQuantity("100001", 0);
        } catch (const PosError&) {
            threwZero = true;
        }
        assert(threwZero);

        // exceed stock throws PosError
        shop.orderService.addItem("100001", 1);
        bool threwStock = false;
        try {
            shop.orderService.updateItemQuantity("100001", 500);
        } catch (const PosError&) {
            threwStock = true;
        }
        assert(threwStock);
    }

    // ---------- hold and resume order ----------
    {
        Shop shop;
        shop.orderService.start();
        shop.orderService.setCustomer("Held Customer", "03001234567");
        shop.orderService.addItem("100001", 3);
        assert(!shop.orderService.hasHeldOrders());

        // Hold order
        shop.orderService.holdCurrentOrder();
        assert(shop.orderService.hasHeldOrders());
        assert(shop.orderService.getHeldOrders().size() == 1);
        assert(shop.orderService.hasActiveDraft());
        assert(shop.orderService.draft().lines.empty());

        // Start a new intermediate order
        shop.orderService.start();
        shop.orderService.setCustomer("New Customer", "03119876543");
        shop.orderService.addItem("100002", 2.5);
        assert(shop.orderService.draft().customerName == "New Customer");

        // Resume the held order
        shop.orderService.resumeHeldOrder(0);
        assert(!shop.orderService.hasHeldOrders());
        assert(shop.orderService.hasActiveDraft());
        assert(shop.orderService.draft().customerName == "Held Customer");
        assert(shop.orderService.draft().lines.size() == 1);
        assert(shop.orderService.draft().lines[0].quantity == 3);
    }

    cout << "test_order_service: all passed" << endl;
    return 0;
}