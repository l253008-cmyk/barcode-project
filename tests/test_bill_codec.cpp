#include "pos/storage/BillCodec.hpp"
#include "pos/common/PosError.hpp"
#include "test_utils.hpp"
#include <string>
#include <vector>
using namespace std;

// Joins the tokens of a record like the file does: spaces and # at the end.
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

static BillLine makeLine(const string& acronym, const char* price, const char* discount,
    double quantity) {
    BillLine line;
    line.acronym = acronym;
    line.unitPrice = Money::parse(price);
    line.discount = Money::parse(discount);
    line.quantity = quantity;
    return line;
}

// The golden bill of the proposal.
static Bill goldenBill() {
    Bill bill;
    bill.id = "000000045";
    bill.customerName = "Ali Raza";
    bill.orderId = "00012314";
    bill.lines.push_back(makeLine("TEA", "350.00", "10.00", 2));
    bill.lines.push_back(makeLine("SUGAR", "180.00", "0.00", 1.5));
    bill.totalWithoutTax = Money::parse("970.00");
    bill.totalTax = Money::parse("35.00");
    bill.totalDiscount = Money::parse("20.00");
    bill.totalPayable = Money::parse("985.00");
    bill.status = "paid";
    return bill;
}

// A record with 13 tokens, built from the golden line, to be changed by the tests.
static vector<string> goldenRecord() {
    return { "000000045", "Ali", "Raza", "00012314", "TEA", "350.00", "10.00", "2",
             "970.00", "35.00", "20.00", "985.00", "paid" };
}

int main() {
    bool ok = true;

    // 1. Writing: the lines must be exactly the golden lines
    vector<vector<string>> records = BillCodec::toRecords(goldenBill());
    ok &= checkResult("One record for each bill line", "2", to_string(records.size()));
    ok &= checkResult("Golden line 1",
        "000000045 Ali Raza 00012314 TEA 350.00 10.00 2 970.00 35.00 20.00 985.00 paid#",
        toFileLine(records[0]));
    ok &= checkResult("Golden line 2",
        "000000045 Ali Raza 00012314 SUGAR 180.00 0.00 1.5 970.00 35.00 20.00 985.00 paid#",
        toFileLine(records[1]));

    // 2. Reading the same records gives the same bill back
    vector<Bill> bills = BillCodec::fromRecords(records);
    ok &= checkResult("Read: one bill", "1", to_string(bills.size()));
    ok &= checkTrue("Read: same bill as written", bills.size() == 1 && bills[0] == goldenBill());

    // 3. Names: one word, three words and no name
    Bill oneWord = goldenBill();
    oneWord.customerName = "Sara";
    ok &= checkTrue("Round trip: one-word name",
        BillCodec::fromRecords(BillCodec::toRecords(oneWord))[0] == oneWord);

    Bill threeWords = goldenBill();
    threeWords.customerName = "Muhammad Ali Khan";
    ok &= checkTrue("Round trip: three-word name",
        BillCodec::fromRecords(BillCodec::toRecords(threeWords))[0] == threeWords);

    Bill noName = goldenBill();
    noName.customerName = "";
    vector<vector<string>> noNameRecords = BillCodec::toRecords(noName);
    ok &= checkResult("No name is written as -", "-", noNameRecords[0][1]);
    ok &= checkTrue("Round trip: no name", BillCodec::fromRecords(noNameRecords)[0] == noName);

    // 4. Two bills in one file
    Bill other = goldenBill();
    other.id = "000000046";
    other.lines.pop_back();
    vector<vector<string>> both = BillCodec::toRecords(goldenBill());
    vector<vector<string>> otherRecords = BillCodec::toRecords(other);
    both.insert(both.end(), otherRecords.begin(), otherRecords.end());
    vector<Bill> two = BillCodec::fromRecords(both);
    ok &= checkResult("Two bills are separated", "2", to_string(two.size()));
    ok &= checkResult("First bill has 2 lines", "2", to_string(two[0].lines.size()));
    ok &= checkResult("Second bill has 1 line", "1", to_string(two[1].lines.size()));

    // 5. Bad records must be refused with a PosError
    ok &= checkThrowsAs<PosError>("Too few fields", [&]() {
        vector<string> record = goldenRecord();
        record.resize(11);
        BillCodec::fromRecords({ record });
        });
    ok &= checkThrowsAs<PosError>("Bill id is not 9 digits", [&]() {
        vector<string> record = goldenRecord();
        record[0] = "45";
        BillCodec::fromRecords({ record });
        });
    ok &= checkThrowsAs<PosError>("Order id is not 8 digits", [&]() {
        vector<string> record = goldenRecord();
        record[3] = "1234";
        BillCodec::fromRecords({ record });
        });
    ok &= checkThrowsAs<PosError>("Bad money", [&]() {
        vector<string> record = goldenRecord();
        record[5] = "35x";
        BillCodec::fromRecords({ record });
        });
    ok &= checkThrowsAs<PosError>("Bad quantity", [&]() {
        vector<string> record = goldenRecord();
        record[7] = "two";
        BillCodec::fromRecords({ record });
        });
    ok &= checkThrowsAs<PosError>("Zero quantity", [&]() {
        vector<string> record = goldenRecord();
        record[7] = "0";
        BillCodec::fromRecords({ record });
        });
    ok &= checkThrowsAs<PosError>("Lines of one bill with different totals", [&]() {
        vector<string> first = goldenRecord();
        vector<string> second = goldenRecord();
        second[11] = "999.00";
        BillCodec::fromRecords({ first, second });
        });

    // 6. Bad bills must be refused when writing
    ok &= checkThrowsAs<PosError>("Bill without lines", [&]() {
        Bill bill = goldenBill();
        bill.lines.clear();
        BillCodec::toRecords(bill);
        });
    ok &= checkThrowsAs<PosError>("Bill id is not 9 digits (write)", [&]() {
        Bill bill = goldenBill();
        bill.id = "12";
        BillCodec::toRecords(bill);
        });

    return ok ? 0 : 1;
}