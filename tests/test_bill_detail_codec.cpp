// test_bill_detail_codec.cpp                         Owner: Person 2
#include "pos/storage/BillDetailCodec.hpp"
#include "pos/common/PosError.hpp"
#include "test_utils.hpp"
#include <iostream>

using namespace std;

int main() {
    bool ok = true;

    // 1. Valid line
    BillDetailLine line{ "000000001", "TEA", Money::parse("350.00"), 2.0, Money::parse("20.00") };
    auto tokens = BillDetailCodec::format(line);
    ok &= checkResult("5 tokens", "5", to_string(tokens.size()));
    ok &= checkResult("Token 0 Bill ID", "000000001", tokens[0]);
    ok &= checkResult("Token 1 Acronym", "TEA", tokens[1]);
    ok &= checkResult("Token 2 Price", "350.00", tokens[2]);
    ok &= checkResult("Token 3 Qty", "2", tokens[3]);
    ok &= checkResult("Token 4 Disc", "20.00", tokens[4]);

    auto parsed = BillDetailCodec::parse(tokens);
    ok &= checkResult("Parsed Bill ID", line.billId, parsed.billId);
    ok &= checkResult("Parsed Acronym", line.acronym, parsed.acronym);
    ok &= checkResult("Parsed Price", line.unitPrice.toString(), parsed.unitPrice.toString());
    ok &= checkTrue("Parsed Qty", parsed.quantity == 2.0);
    ok &= checkResult("Parsed Disc", line.discount.toString(), parsed.discount.toString());

    // 2. Reject bad tokens
    ok &= checkThrowsAs<PosError>("Too few tokens", [&]() {
        BillDetailCodec::parse({ "000000001", "TEA", "350.00" });
        });

    ok &= checkThrowsAs<PosError>("Bad bill ID length", [&]() {
        BillDetailCodec::parse({ "123", "TEA", "350.00", "2", "0.00" });
        });

    if (ok) {
        cout << "test_bill_detail_codec: all passed\n";
        return 0;
    }
    cout << "test_bill_detail_codec: some tests failed\n";
    return 1;
}