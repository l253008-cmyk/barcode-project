// Tests for IdGenerator (P3), with fake repositories and a fake clock so the
// result does not depend on files or on today's date.
#include <iostream>
#include <string>

#include "pos/common/IClock.hpp"
#include "pos/common/PosError.hpp"
#include "pos/domain/Bill.hpp"
#include "pos/domain/Order.hpp"
#include "pos/services/IdGenerator.hpp"
#include "pos/storage/IBillRepository.hpp"
#include "pos/storage/IOrderRepository.hpp"

static int failures = 0;

#define CHECK(condition)                                                              \
    do {                                                                              \
        if (!(condition)) {                                                           \
            std::cout << "FAIL (line " << __LINE__ << "): " #condition << std::endl;  \
            ++failures;                                                               \
        }                                                                             \
    } while (0)

#define CHECK_THROWS(statement)                                                       \
    do {                                                                              \
        bool threw = false;                                                           \
        try { statement; } catch (const pos::PosError&) { threw = true; }             \
        if (!threw) {                                                                 \
            std::cout << "FAIL (line " << __LINE__ << "): no PosError from " #statement \
                      << std::endl;                                                   \
            ++failures;                                                               \
        }                                                                             \
    } while (0)

using namespace pos;

namespace {

class FakeClock : public IClock {
public:
    explicit FakeClock(const std::string& date) : date_(date) {}
    void setDate(const std::string& date) { date_ = date; }
    std::string today() const override { return date_; }
    std::string now() const override { return "10:32:05"; }
    long long millis() const override { return 0; }

private:
    std::string date_;
};

class FakeOrderRepository : public IOrderRepository {
public:
    void add(const Order& order) override { last_ = order.id; }
    std::optional<Order> findById(const std::string&) const override { return std::nullopt; }
    std::vector<Order> all() const override { return {}; }
    void update(const Order&) override {}
    std::string lastId() const override { return last_; }
    void setLast(const std::string& id) { last_ = id; }

private:
    std::string last_;
};

class FakeBillRepository : public IBillRepository {
public:
    void add(const Bill& bill) override { last_ = bill.id; }
    void replace(const Bill&) override {}
    std::optional<Bill> findByOrderId(const std::string&) const override { return std::nullopt; }
    std::vector<Bill> all() const override { return {}; }
    std::string lastId() const override { return last_; }
    void setLast(const std::string& id) { last_ = id; }

private:
    std::string last_;
};

}  // namespace

static void testFirstIds() {
    FakeOrderRepository orders;
    FakeBillRepository bills;
    FakeClock clock("2026-11-14");
    IdGenerator ids(orders, bills, clock);

    CHECK(ids.nextOrderId() == "00000114");   // order 1 on the 14th
    CHECK(ids.nextBillId() == "000000001");
}

static void testIdsFollowTheLastOne() {
    FakeOrderRepository orders;
    FakeBillRepository bills;
    FakeClock clock("2026-11-14");
    IdGenerator ids(orders, bills, clock);

    orders.setLast("00012314");                // order 123 on the 14th
    bills.setLast("000000045");
    CHECK(ids.nextOrderId() == "00012414");    // order 124
    CHECK(ids.nextBillId() == "000000046");
}

static void testDayComesFromTheClock() {
    FakeOrderRepository orders;
    FakeBillRepository bills;
    FakeClock clock("2026-11-14");
    IdGenerator ids(orders, bills, clock);

    orders.setLast("00012314");
    clock.setDate("2026-12-03");
    CHECK(ids.nextOrderId() == "00012403");    // number keeps growing, day changes
}

static void testSameIdUntilSaved() {
    FakeOrderRepository orders;
    FakeBillRepository bills;
    FakeClock clock("2026-11-14");
    IdGenerator ids(orders, bills, clock);

    std::string first = ids.nextOrderId();
    CHECK(ids.nextOrderId() == first);         // nothing saved yet
    Order order;
    order.id = first;
    orders.add(order);
    CHECK(ids.nextOrderId() == "00000214");
}

static void testBadStoredIdsAndLimits() {
    FakeOrderRepository orders;
    FakeBillRepository bills;
    FakeClock clock("2026-11-14");
    IdGenerator ids(orders, bills, clock);

    orders.setLast("12ab");
    CHECK_THROWS(ids.nextOrderId());
    bills.setLast("45");
    CHECK_THROWS(ids.nextBillId());

    orders.setLast("99999914");                // no room for another order number
    CHECK_THROWS(ids.nextOrderId());
    bills.setLast("999999999");
    CHECK_THROWS(ids.nextBillId());

    orders.setLast("");
    clock.setDate("14/11/2026");               // a clock that gives the wrong format
    CHECK_THROWS(ids.nextOrderId());
}

int main() {
    testFirstIds();
    testIdsFollowTheLastOne();
    testDayComesFromTheClock();
    testSameIdUntilSaved();
    testBadStoredIdsAndLimits();

    if (failures == 0) {
        std::cout << "test_id_generator: all checks passed" << std::endl;
        return 0;
    }
    std::cout << "test_id_generator: " << failures << " check(s) failed" << std::endl;
    return 1;
}
