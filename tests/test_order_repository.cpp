// Tests for FileOrderRepository and FileOrderDetailRepository (P3). They write
// to scratch files in the current folder and delete them when done.
#include <fstream>
#include <iostream>
#include <string>

#include "pos/common/PosError.hpp"
#include "pos/common/TextRecordFile.hpp"
#include "pos/domain/Order.hpp"
#include "pos/storage/FileOrderDetailRepository.hpp"
#include "pos/storage/FileOrderRepository.hpp"

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

static const char* kOrderPath = "scratch_orders.txt";
static const char* kDetailPath = "scratch_order_details.txt";

static std::string readAllText(const char* path) {
    std::ifstream in(path, std::ios::binary);
    std::string all, line;
    while (std::getline(in, line)) all += line + "\n";
    return all;
}

static void cleanUp() {
    TextRecordFile(kOrderPath).removeFromDisk();
    TextRecordFile(kDetailPath).removeFromDisk();
}

static Order makeOrder(const std::string& id) {
    Order order;
    order.id = id;
    order.date = "2026-11-14";
    order.time = "10:32:05";
    order.customerName = "Ali Raza";
    order.contact = "0300-1234567";
    order.status = OrderStatus::Paid;
    order.payment = Money::parse("985.00");
    order.mode = PaymentMode::Cash;
    return order;
}

static OrderLine makeLine(int productId, const std::string& barcode, double quantity) {
    OrderLine line;
    line.productId = productId;
    line.barcode = barcode;
    line.quantity = quantity;
    return line;
}

static void testOrderRepository() {
    cleanUp();
    FileOrderRepository repo(kOrderPath);

    CHECK(repo.lastId().empty());           // a new file has no orders
    CHECK(repo.all().empty());

    repo.add(makeOrder("00012314"));
    CHECK(readAllText(kOrderPath) ==
        "00012314 2026-11-14 10:32:05 Ali Raza 0300-1234567 paid 985.00 cash -\x23\n");
    CHECK(repo.lastId() == "00012314");

    Order second = makeOrder("00012415");
    second.customerName = "Sara Khan";
    second.status = OrderStatus::OnHold;
    second.mode = PaymentMode::Card;
    second.merchant = "HBL";
    repo.add(second);
    CHECK(repo.lastId() == "00012415");

    std::vector<Order> all = repo.all();
    CHECK(all.size() == 2);
    CHECK(all[0].id == "00012314");
    CHECK(all[0].customerName == "Ali Raza");
    CHECK(all[1].customerName == "Sara Khan");
    CHECK(all[1].status == OrderStatus::OnHold);
    CHECK(all[1].mode == PaymentMode::Card);
    CHECK(all[1].merchant == "HBL");

    // A second repository object on the same file sees the same data.
    FileOrderRepository again(kOrderPath);
    CHECK(again.lastId() == "00012415");

    // A bad order is refused and leaves the file alone.
    Order bad = makeOrder("12");
    CHECK_THROWS(repo.add(bad));
    CHECK(repo.all().size() == 2);
    cleanUp();
}

static void testDamagedOrderFile() {
    cleanUp();
    {
        std::ofstream out(kOrderPath);
        out << "00012314 2026-11-14 10:32:05 Ali Raza 0300 paid 985.00 cash -\n";  // no marker
    }
    FileOrderRepository repo(kOrderPath);
    CHECK_THROWS(repo.lastId());
    cleanUp();
}

static void testDetailRepository() {
    cleanUp();
    FileOrderDetailRepository repo(kDetailPath);
    CHECK(repo.all().empty());

    repo.add("00012314", makeLine(1, "100001", 2));
    repo.add("00012314", makeLine(2, "100002", 1.5));
    CHECK(readAllText(kDetailPath) == "00012314 1 100001 2\x23\n00012314 2 100002 1.5\x23\n");

    std::vector<IOrderDetailRepository::Entry> rows = repo.all();
    CHECK(rows.size() == 2);
    CHECK(rows[0].orderId == "00012314" && rows[0].line.productId == 1 && rows[0].line.quantity == 2);
    CHECK(rows[1].line.barcode == "100002" && rows[1].line.quantity == 1.5);

    CHECK_THROWS(repo.add("00012314", makeLine(3, "100003", 0)));   // zero quantity
    CHECK(repo.all().size() == 2);
    cleanUp();
}

int main() {
    testOrderRepository();
    testDamagedOrderFile();
    testDetailRepository();

    if (failures == 0) {
        std::cout << "test_order_repository: all checks passed" << std::endl;
        return 0;
    }
    std::cout << "test_order_repository: " << failures << " check(s) failed" << std::endl;
    return 1;
}