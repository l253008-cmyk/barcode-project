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
        for (const auto& p : products) {
            if (p.barcode == barcode) return p;
        }
        return nullopt;
    }

    optional<Product> findById(int id) const override {
        for (const auto& p : products) {
            if (p.id == id) return p;
        }
        return nullopt;
    }

    void updateQuantity(int id, double newQuantity) override {
        for (auto& p : products) {
            if (p.id == id) {
                p.availableQty = newQuantity;
                return;
            }
        }
    }

    void update(const Product& product) override {
        for (auto& p : products) {
            if (p.id == product.id) {
                p = product;
                return;
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
        orders.push_back(order);
    }

    optional<Order> findById(const string& id) const override {
        for (const auto& o : orders) {
            if (o.id == id) return o;
        }
        return nullopt;
    }

    vector<Order> all() const override {
        return orders;
    }

    void update(const Order& order) override {
        for (auto& o : orders) {
            if (o.id == order.id) {
                o = order;
                return;
            }
        }
    }

    string lastId() const override {
        if (orders.empty()) {
            return "";
        }
        return orders.back().id;
    }
};

class FakeOrderDetailRepository : public IOrderDetailRepository {
public:
    vector<Entry> rows;

    void add(const string& orderId, const OrderLine& line) override {
        rows.push_back(Entry{orderId, line});
    }

    vector<OrderLine> findByOrderId(const string& orderId) const override {
        vector<OrderLine> lines;
        for (const auto& r : rows) {
            if (r.orderId == orderId) {
                lines.push_back(r.line);
            }
        }
        return lines;
    }

    void replaceForOrder(const string& orderId, const vector<OrderLine>& lines) override {
        vector<Entry> kept;
        for (const auto& r : rows) {
            if (r.orderId != orderId) kept.push_back(r);
        }
        rows = kept;
        for (const auto& l : lines) {
            rows.push_back(Entry{orderId, l});
        }
    }

    vector<Entry> all() const override {
        return rows;
    }
};

class FakeBillRepository : public IBillRepository {
public:
    vector<Bill> bills;

    void add(const Bill& b) override {
        bills.push_back(b);
    }

    void replace(const Bill& b) override {
        for (auto& x : bills) {
            if (x.id == b.id) {
                x = b;
                return;
            }
        }
    }

    optional<Bill> findByOrderId(const string& orderId) const override {
        for (const auto& b : bills) {
            if (b.orderId == orderId) return b;
        }
        return nullopt;
    }

    vector<Bill> all() const override {
        return bills;
    }

    string lastId() const override {
        if (bills.empty()) return "";
        return bills.back().id;
    }
};

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

static Product makeTea() {
    Product tea = { 1, "10", "100001", "TEA", "Tetley Tea Bags 25", MeasurementUnit::Count,
                   Money::parse("350.00"), Money::parse("17.50"), Money::parse("10.00"),
                   40, "data/barcodes/100001.bmp" };
    return tea;
}

static Product makeSugar() {
    Product sugar = { 2, "10", "100002", "SUGAR", "Sugar Loose", MeasurementUnit::Weight,
                     Money::parse("180.00"), Money::parse("0.00"), Money::parse("0.00"),
                     25.5, "data/barcodes/100002.bmp" };
    return sugar;
}

static Product makeBiscuit() {
    Product biscuit = { 3, "10", "100003", "BISCUIT", "Sooper Biscuit", MeasurementUnit::Count,
                       Money::parse("60.00"), Money::parse("3.00"), Money::parse("5.00"),
                       100, "data/barcodes/100003.bmp" };
    return biscuit;
}

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
        idGenerator(orders, bills, fakeClock),
        orderService(products, productService, orders, details, idGenerator, fakeClock) {
        products.products.push_back(makeTea());
        products.products.push_back(makeSugar());
        products.products.push_back(makeBiscuit());

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
        orders.orders.push_back(earlier);
    }
};

static bool addItemThrows(OrderService& service, const string& barcode, double quantity) {
    try {
        service.addItem(barcode, quantity);
    } catch (const PosError&) {
        return true;
    }
    return false;
}

static bool removeItemThrows(OrderService& service, const string& barcode) {
    try {
        service.removeItem(barcode);
    } catch (const PosError&) {
        return true;
    }
    return false;
}

static bool setCustomerThrows(OrderService& service, const string& name, const string& contact) {
    try {
        service.setCustomer(name, contact);
    } catch (const PosError&) {
        return true;
    }
    return false;
}

static bool setPaymentThrows(OrderService& service, PaymentMode mode, const string& merchant) {
    try {
        service.setPayment(mode, merchant);
    } catch (const PosError&) {
        return true;
    }
    return false;
}

static bool finalizeThrows(OrderService& service) {
    try {
        service.finalize();
    } catch (const PosError&) {
        return true;
    }
    return false;
}

static bool draftThrows(const OrderService& service) {
    try {
        service.draft();
    } catch (const PosError&) {
        return true;
    }
    return false;
}

int main() {
    // 1. The order from proposal
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
        assert(saved.lines[0].unitPrice.toString() == "350.00");
        assert(saved.lines[0].tax.toString() == "17.50");

        assert(shop.orders.orders.size() == 2);
        assert(shop.orders.orders[1].id == "00012314");

        assert(shop.details.rows.size() == 2);
        assert(shop.details.rows[0].orderId == "00012314");
        assert(shop.details.rows[0].line.productId == 1);
        assert(shop.details.rows[0].line.barcode == "100001");
        assert(shop.details.rows[0].line.quantity == 2);
        assert(shop.details.rows[1].orderId == "00012314");
        assert(shop.details.rows[1].line.productId == 2);
        assert(shop.details.rows[1].line.quantity == 1.5);

        assert(shop.products.findById(1).value().availableQty == 38);
        assert(shop.products.findById(2).value().availableQty == 24);
        assert(shop.products.findById(3).value().availableQty == 100);

        assert(draftThrows(shop.orderService));
        assert(finalizeThrows(shop.orderService));

        shop.orderService.start();
        shop.orderService.setCustomer("Sara Khan", "");
        shop.orderService.setPayment(PaymentMode::Wallet, "Easypaisa");
        shop.orderService.addItem("100003", 1);
        Order second = shop.orderService.finalize();
        assert(second.id == "00012414");
    }

    // 2. Tax and discount
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

    // 3. Stock rule
    {
        Shop shop;
        shop.orderService.start();
        assert(addItemThrows(shop.orderService, "100001", 41));
        assert(shop.orderService.draft().lines.empty());
        shop.orderService.addItem("100001", 40);
        assert(shop.orderService.draft().lines.size() == 1);
        assert(shop.orderService.draft().lines[0].quantity == 40);
    }

    // 4. Update quantity
    {
        Shop shop;
        shop.orderService.start();
        shop.orderService.addItem("100001", 2);
        assert(shop.orderService.draft().lines[0].quantity == 2);

        shop.orderService.updateItemQuantity("100001", 5);
        assert(shop.orderService.draft().lines[0].quantity == 5);

        shop.orderService.updateItemQuantity("100001", 1);
        assert(shop.orderService.draft().lines[0].quantity == 1);

        bool threwZero = false;
        try {
            shop.orderService.updateItemQuantity("100001", 0);
        } catch (const PosError&) {
            threwZero = true;
        }
        assert(threwZero);
    }

    // 5. Hold and resume order
    {
        Shop shop;
        shop.orderService.start();
        shop.orderService.setCustomer("Held Customer", "03001234567");
        shop.orderService.addItem("100001", 3);
        assert(!shop.orderService.hasHeldOrders());

        shop.orderService.holdCurrentOrder();
        assert(shop.orderService.hasHeldOrders());
        assert(shop.orderService.getHeldOrders().size() == 1);
        assert(shop.orderService.hasActiveDraft());
        assert(shop.orderService.draft().lines.empty());

        shop.orderService.start();
        shop.orderService.setCustomer("New Customer", "03119876543");
        shop.orderService.addItem("100002", 2.5);
        assert(shop.orderService.draft().customerName == "New Customer");

        shop.orderService.resumeHeldOrder(0);
        assert(!shop.orderService.hasHeldOrders());
        assert(shop.orderService.hasActiveDraft());
        assert(shop.orderService.draft().customerName == "Held Customer");
        assert(shop.orderService.draft().lines.size() == 1);
        assert(shop.orderService.draft().lines[0].quantity == 3);
    }

    // 6. Order editing (7-day rule & manager approval)
    {
        Shop shop;
        // 00012214 was paid on 2026-11-14 (today in fakeClock is 2026-11-14 -> 0 days elapsed)
        // Without manager approval: must throw
        bool threwNoApproval = false;
        try {
            shop.orderService.loadOrderForEdit("00012214", false);
        } catch (const PosError&) {
            threwNoApproval = true;
        }
        assert(threwNoApproval);

        // With manager approval: succeeds
        shop.orderService.loadOrderForEdit("00012214", true);
        assert(shop.orderService.isEditingExisting());
        assert(shop.orderService.draft().id == "00012214");

        // Can add items and finalizeEdit
        shop.orderService.addItem("100003", 2);
        Order edited = shop.orderService.finalizeEdit();
        assert(edited.id == "00012214");
    }

    cout << "test_order_service: all passed" << endl;
    return 0;
}