// Tests for OrderCodec and OrderDetailCodec (P3), using the golden data from
// section 10 of the proposal.
#include <iostream>
#include <string>

#include "pos/common/PosError.hpp"
#include "pos/domain/Order.hpp"
#include "pos/storage/OrderCodec.hpp"
#include "pos/storage/OrderDetailCodec.hpp"

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

static const char* kGoldenOrder =
    "00012314 2026-11-14 10:32:05 Ali Raza 0300-1234567 paid 985.00 cash -#";

static Order goldenOrder() {
    Order order;
    order.id = "00012314";
    order.date = "2026-11-14";
    order.time = "10:32:05";
    order.customerName = "Ali Raza";
    order.contact = "0300-1234567";
    order.status = OrderStatus::Paid;
    order.payment = Money::parse("985.00");
    order.mode = PaymentMode::Cash;
    order.merchant = "";
    return order;
}

static OrderLine teaLine() {
    OrderLine line;
    line.productId = 1;
    line.barcode = "100001";
    line.acronym = "TEA";
    line.unitPrice = Money::parse("350.00");
    line.tax = Money::parse("17.50");
    line.discount = Money::parse("10.00");
    line.quantity = 2;
    return line;
}

static OrderLine sugarLine() {
    OrderLine line;
    line.productId = 2;
    line.barcode = "100002";
    line.acronym = "SUGAR";
    line.unitPrice = Money::parse("180.00");
    line.quantity = 1.5;
    return line;
}

static void testParseGoldenOrder() {
    Order o = OrderCodec::parseLine(kGoldenOrder);
    CHECK(o.id == "00012314");
    CHECK(o.date == "2026-11-14");
    CHECK(o.time == "10:32:05");
    CHECK(o.customerName == "Ali Raza");
    CHECK(o.contact == "0300-1234567");
    CHECK(o.status == OrderStatus::Paid);
    CHECK(o.payment.toString() == "985.00");
    CHECK(o.mode == PaymentMode::Cash);
    CHECK(o.merchant.empty());   // "-" means empty
    CHECK(o.lines.empty());
}

static void testFormatGoldenOrder() {
    CHECK(OrderCodec::format(goldenOrder()) == kGoldenOrder);
}

static void testNameWithManyWords() {
    Order o = OrderCodec::parseLine(
        "00000114 2026-11-14 09:00:00 Muhammad Ali Raza Khan 0300-1 on-hold 0.00 wallet JazzCash#");
    CHECK(o.customerName == "Muhammad Ali Raza Khan");
    CHECK(o.contact == "0300-1");
    CHECK(o.status == OrderStatus::OnHold);
    CHECK(o.mode == PaymentMode::Wallet);
    CHECK(o.merchant == "JazzCash");
    CHECK(OrderCodec::format(o) ==
          "00000114 2026-11-14 09:00:00 Muhammad Ali Raza Khan 0300-1 on-hold 0.00 wallet JazzCash#");
}

static void testEmptyFieldsUseDash() {
    Order o = goldenOrder();
    o.contact = "";
    o.merchant = "";
    std::string line = OrderCodec::format(o);
    CHECK(line == "00012314 2026-11-14 10:32:05 Ali Raza - paid 985.00 cash -#");
    Order back = OrderCodec::parseLine(line);
    CHECK(back.contact.empty());
    CHECK(back.customerName == "Ali Raza");

    o.customerName = "";
    Order walkIn = OrderCodec::parseLine(OrderCodec::format(o));
    CHECK(walkIn.customerName.empty());
}

static void testStatusAndModeNames() {
    CHECK(toString(OrderStatus::InProgress) == "in-progress");
    CHECK(toString(OrderStatus::Archived) == "archived");
    CHECK(toString(PaymentMode::Card) == "card");
    CHECK(orderStatusFromString("on-hold") == OrderStatus::OnHold);
    CHECK(paymentModeFromString("wallet") == PaymentMode::Wallet);
    CHECK_THROWS(orderStatusFromString("Paid"));
    CHECK_THROWS(paymentModeFromString("cheque"));
}

static void testBadOrderLines() {
    CHECK_THROWS(OrderCodec::parseLine("00012314 2026-11-14 10:32:05 Ali 0300 paid 985.00 cash#"));   // 8 fields
    CHECK_THROWS(OrderCodec::parseLine("0012314 2026-11-14 10:32:05 Ali Raza 0300 paid 985.00 cash -#"));  // 7-digit id
    CHECK_THROWS(OrderCodec::parseLine("00012314 2026-13-14 10:32:05 Ali Raza 0300 paid 985.00 cash -#")); // month 13
    CHECK_THROWS(OrderCodec::parseLine("00012314 2026-11-14 25:32:05 Ali Raza 0300 paid 985.00 cash -#")); // hour 25
    CHECK_THROWS(OrderCodec::parseLine("00012314 2026-11-14 10:32:05 Ali Raza 0300 done 985.00 cash -#")); // bad status
}

static void testFormatRefusesUnsafeFields() {
    Order o = goldenOrder();
    o.contact = "0300 1234567";   // a space would break the parsing rule
    CHECK_THROWS(OrderCodec::format(o));

    o = goldenOrder();
    o.merchant = "Jazz Cash";
    CHECK_THROWS(OrderCodec::format(o));

    o = goldenOrder();
    o.id = "123";
    CHECK_THROWS(OrderCodec::format(o));

    o = goldenOrder();
    o.customerName = "Ali #1";
    CHECK_THROWS(OrderCodec::format(o));
}

static void testOrderDetailGolden() {
    CHECK(OrderDetailCodec::format("00012314", teaLine()) == "00012314 1 100001 2#");
    CHECK(OrderDetailCodec::format("00012314", sugarLine()) == "00012314 2 100002 1.5#");

    OrderDetailRecord r = OrderDetailCodec::parseLine("00012314 2 100002 1.5#");
    CHECK(r.orderId == "00012314");
    CHECK(r.productId == 2);
    CHECK(r.barcode == "100002");
    CHECK(r.quantity == 1.5);
}

static void testOrderDetailRejectsBadData() {
    CHECK_THROWS(OrderDetailCodec::parseLine("00012314 1 100001#"));              // 3 fields
    CHECK_THROWS(OrderDetailCodec::parseLine("00012314 x 100001 2#"));            // product id
    CHECK_THROWS(OrderDetailCodec::parseLine("00012314 1 100001 two#"));          // quantity
    CHECK_THROWS(OrderDetailCodec::parseLine("00012314 1 100001 0#"));            // zero quantity
    CHECK_THROWS(OrderDetailCodec::parseLine("1234 1 100001 2#"));                // order id

    OrderLine line = teaLine();
    line.quantity = 0;
    CHECK_THROWS(OrderDetailCodec::format("00012314", line));
    line = teaLine();
    line.barcode = "10 0001";
    CHECK_THROWS(OrderDetailCodec::format("00012314", line));
}

int main() {
    testParseGoldenOrder();
    testFormatGoldenOrder();
    testNameWithManyWords();
    testEmptyFieldsUseDash();
    testStatusAndModeNames();
    testBadOrderLines();
    testFormatRefusesUnsafeFields();
    testOrderDetailGolden();
    testOrderDetailRejectsBadData();

    if (failures == 0) {
        std::cout << "test_order_codec: all checks passed" << std::endl;
        return 0;
    }
    std::cout << "test_order_codec: " << failures << " check(s) failed" << std::endl;
    return 1;
}
