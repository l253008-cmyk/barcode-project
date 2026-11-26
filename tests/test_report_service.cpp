// test_report_service.cpp                            Owner: Person 4
#include "pos/services/ReportService.hpp"
#include "pos/storage/IBillRepository.hpp"
#include "pos/storage/IOrderDetailRepository.hpp"
#include "pos/storage/IOrderRepository.hpp"
#include "pos/storage/IProductRepository.hpp"
#include "test_utils.hpp"
#include <iostream>
#include <vector>

using namespace std;

class FakeProducts : public IProductRepository {
public:
    vector<Product> list;
    vector<Product> all() const override { return list; }
    optional<Product> findByBarcode(const string& bc) const override {
        for (const auto& p : list) if (p.barcode == bc) return p;
        return nullopt;
    }
    optional<Product> findById(int id) const override {
        for (const auto& p : list) if (p.id == id) return p;
        return nullopt;
    }
    void updateQuantity(int, double) override {}
    void update(const Product&) override {}
    void add(const Product& p) override { list.push_back(p); }
};

class FakeOrders : public IOrderRepository {
public:
    vector<Order> list;
    void add(const Order& o) override { list.push_back(o); }
    optional<Order> findById(const string& id) const override {
        for (const auto& o : list) if (o.id == id) return o;
        return nullopt;
    }
    vector<Order> all() const override { return list; }
    void update(const Order&) override {}
    string lastId() const override { return list.empty() ? "" : list.back().id; }
};

class FakeDetails : public IOrderDetailRepository {
public:
    vector<Entry> list;
    void add(const string& orderId, const OrderLine& l) override { list.push_back({ orderId, l }); }
    vector<OrderLine> findByOrderId(const string& id) const override {
        vector<OrderLine> res;
        for (const auto& e : list) if (e.orderId == id) res.push_back(e.line);
        return res;
    }
    void replaceForOrder(const string&, const vector<OrderLine>&) override {}
    vector<Entry> all() const override { return list; }
};

class FakeBills : public IBillRepository {
public:
    vector<Bill> list;
    void add(const Bill& b) override { list.push_back(b); }
    void replace(const Bill&) override {}
    optional<Bill> findByOrderId(const string& id) const override {
        for (const auto& b : list) if (b.orderId == id) return b;
        return nullopt;
    }
    vector<Bill> all() const override { return list; }
    string lastId() const override { return list.empty() ? "" : list.back().id; }
};

int main() {
    bool ok = true;

    FakeOrders orders;
    FakeDetails details;
    FakeBills bills;
    FakeProducts products;

    // Seed products
    products.list.push_back({ 1, "10", "100001", "TEA", "Tea", MeasurementUnit::Count, Money::parse("100.00"), Money(), Money(), 50, "" });
    products.list.push_back({ 2, "10", "100002", "SUGAR", "Sugar", MeasurementUnit::Weight, Money::parse("200.00"), Money(), Money(), 10, "" });

    // Seed orders
    Order ord1;
    ord1.id = "00000001";
    ord1.customerName = "Ali";
    ord1.status = OrderStatus::Paid;
    ord1.date = "2026-11-20";
    ord1.payment = Money::parse("500.00");
    orders.list.push_back(ord1);

    Order ord2;
    ord2.id = "00000002";
    ord2.customerName = "Sara";
    ord2.status = OrderStatus::InProgress;
    ord2.date = "2026-11-21";
    ord2.payment = Money::parse("300.00");
    orders.list.push_back(ord2);

    // Seed details
    OrderLine l1;
    l1.barcode = "100001";
    l1.acronym = "TEA";
    l1.quantity = 5.0;
    details.list.push_back({ "00000001", l1 });

    // Seed bills
    Bill b1;
    b1.id = "000000001";
    b1.orderId = "00000001";
    b1.customerName = "Ali";
    b1.date = "2026-11-20";
    b1.totalPayable = Money::parse("500.00");
    b1.amountReceived = Money::parse("500.00");
    b1.balanceReturned = Money::parse("0.00");
    b1.status = "paid";
    bills.list.push_back(b1);

    ReportService service(orders, details, bills, products);

    // 1. Incomplete orders
    auto repIncomplete = service.incompleteOrders();
    ok &= checkResult("1 incomplete order", "1", to_string(repIncomplete.rows.size()));
    ok &= checkResult("Order ID", "00000002", repIncomplete.rows[0][0]);

    // 2. Completed orders
    auto repCompleted = service.completedOrders();
    ok &= checkResult("1 completed order", "1", to_string(repCompleted.rows.size()));
    ok &= checkResult("Order ID", "00000001", repCompleted.rows[0][0]);

    // 3. Cash sales
    auto repCash = service.cashSalesByPeriod("2026-11-01", "2026-11-25");
    ok &= checkResult("1 cash sale", "1", to_string(repCash.rows.size()));

    // 4. All sales with discounts
    auto repDiscounts = service.allSalesWithDiscounts();
    ok &= checkResult("1 sale", "1", to_string(repDiscounts.rows.size()));

    // 5. Top sold items
    auto repTopSold = service.top10MostSoldItems();
    ok &= checkResult("1 item sold", "1", to_string(repTopSold.rows.size()));
    ok &= checkResult("Top item is TEA", "TEA", repTopSold.rows[0][1]);

    // 6. Top items by stock
    auto repTopStock = service.top10ByRemainingStock();
    ok &= checkResult("2 items in stock", "2", to_string(repTopStock.rows.size()));
    ok &= checkResult("Highest stock is TEA (50)", "TEA", repTopStock.rows[0][2]);

    if (ok) {
        cout << "test_report_service: all passed\n";
        return 0;
    }
    cout << "test_report_service: some tests failed\n";
    return 1;
}