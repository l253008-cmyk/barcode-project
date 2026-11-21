// test_receipt_formatter.cpp                         Owner: Person 3
#include "pos/services/ReceiptFormatter.hpp"
#include "test_utils.hpp"
#include <iostream>

using namespace std;

int main() {
    bool ok = true;

    Bill bill;
    bill.id = "000000001";
    bill.orderId = "00000118";
    bill.customerName = "Mahnoor";
    bill.date = "2026-11-21";
    bill.time = "14:30:00";
    bill.status = "paid";
    bill.totalWithoutTax = Money::parse("1000.00");
    bill.totalTax = Money::parse("50.00");
    bill.totalDiscount = Money::parse("20.00");
    bill.seasonalDiscount = Money::parse("50.00");
    bill.specialDiscount = Money::parse("20.00");
    bill.totalPayable = Money::parse("960.00");
    bill.amountReceived = Money::parse("1000.00");
    bill.balanceReturned = Money::parse("40.00");

    BillDetailLine line;
    line.billId = bill.id;
    line.acronym = "TEA";
    line.unitPrice = Money::parse("350.00");
    line.quantity = 2.0;
    line.discount = Money::parse("20.00");
    bill.lines.push_back(line);

    auto lines = ReceiptFormatter::lines(bill, "Test Store POS");
    ok &= checkTrue("Has receipt lines", !lines.empty());

    string joined;
    for (const auto& l : lines) {
        joined += l + "\n";
    }

    ok &= checkTrue("Contains shop name", joined.find("Test Store POS") != string::npos);
    ok &= checkTrue("Contains bill ID", joined.find("000000001") != string::npos);
    ok &= checkTrue("Contains customer", joined.find("Mahnoor") != string::npos);
    ok &= checkTrue("Contains TEA", joined.find("TEA") != string::npos);
    ok &= checkTrue("Contains Total Payable", joined.find("960.00") != string::npos);
    ok &= checkTrue("Contains Seasonal Discount", joined.find("Seasonal Discount:  -50.00") != string::npos);
    ok &= checkTrue("Contains Special Discount", joined.find("Special Discount:   -20.00") != string::npos);

    if (ok) {
        cout << "test_receipt_formatter: all passed\n";
        return 0;
    }
    cout << "test_receipt_formatter: some tests failed\n";
    return 1;
}
