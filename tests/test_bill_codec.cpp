#include "pos/storage/BillCodec.hpp"
#include "pos/common/PosError.hpp"
#include "test_utils.hpp"
#include <string>
#include <vector>
using namespace std;

static string toFileLine(const vector<string>& record) {
    string line;
    for (size_t i = 0; i < record.size(); i++) {
        if (i > 0) {
            line += " ";
        }
        line += record[i];
    }
    return line + "#";
}

static Bill goldenBill() {
    Bill bill;
    bill.id = "000000045";
    bill.customerName = "Ali Raza";
    bill.orderId = "00012314";
    bill.totalWithoutTax = Money::parse("970.00");
    bill.totalTax = Money::parse("35.00");
    bill.totalDiscount = Money::parse("20.00");
    bill.totalPayable = Money::parse("985.00");
    bill.amountReceived = Money::parse("1000.00");
    bill.balanceReturned = Money::parse("15.00");
    bill.status = "paid";
    bill.seasonalDiscount = Money::parse("0.00");
    bill.specialDiscount = Money::parse("0.00");
    bill.date = "2026-11-21";
    bill.time = "17:45:00";
    return bill;
}

int main() {
    bool ok = true;

    // 1. Writing: the line must be 14 tokens
    vector<string> record = BillCodec::format(goldenBill());
    ok &= checkResult("15 tokens with 2-word name", "15", to_string(record.size()));
    ok &= checkResult("Golden record line",
        "000000045 Ali Raza 00012314 970.00 35.00 20.00 985.00 1000.00 15.00 paid 0.00 0.00 2026-11-21 17:45:00#",
        toFileLine(record));

    // 2. Reading back gives the same bill
    Bill parsed = BillCodec::parse(record);
    ok &= checkResult("ID parsed", goldenBill().id, parsed.id);
    ok &= checkResult("Customer parsed", goldenBill().customerName, parsed.customerName);
    ok &= checkResult("Total payable parsed", goldenBill().totalPayable.toString(), parsed.totalPayable.toString());
    ok &= checkResult("Amount received parsed", goldenBill().amountReceived.toString(), parsed.amountReceived.toString());
    ok &= checkResult("Date parsed", goldenBill().date, parsed.date);

    // 3. Round-trip single word and multi-word names
    Bill singleWord = goldenBill();
    singleWord.customerName = "Mahnoor";
    ok &= checkResult("Round trip single word", "Mahnoor", BillCodec::parse(BillCodec::format(singleWord)).customerName);

    Bill threeWord = goldenBill();
    threeWord.customerName = "Muhammad Ali Khan";
    ok &= checkResult("Round trip three words", "Muhammad Ali Khan", BillCodec::parse(BillCodec::format(threeWord)).customerName);

    // 4. Bad records
    ok &= checkThrowsAs<PosError>("Too few tokens", [&]() {
        vector<string> bad = { "000000045", "Customer", "00012314" };
        BillCodec::parse(bad);
        });

    ok &= checkThrowsAs<PosError>("Bad ID length", [&]() {
        vector<string> bad = record;
        bad[0] = "45";
        BillCodec::parse(bad);
        });

    return ok ? 0 : 1;
}