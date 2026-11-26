// BillCodec.cpp                                       Owner: Person 2
// Implementation of 14-field BillCodec for Bill.txt.

#include "pos/storage/BillCodec.hpp"
#include "pos/common/PosError.hpp"

using namespace std;

static Money parseMoney(const string& text, const string& fieldName) {
    try {
        Money m = Money::parse(text);
        if (m.paisa() < 0) {
            throw PosError(fieldName + " cannot be negative: '" + text + "'");
        }
        return m;
    }
    catch (const PosError& e) {
        throw PosError(fieldName + " invalid money amount: '" + text + "' (" + e.what() + ")");
    }
}

Bill BillCodec::parse(const vector<string>& tokens) {
    // 1 token for billId + at least 1 token for customerName + 12 fixed tokens = 14 minimum
    int count = static_cast<int>(tokens.size());
    if (count < 14) {
        throw PosError("Bill line requires at least 14 tokens, found " + to_string(count));
    }

    string billId = tokens[0];
    if (billId.size() != 9) {
        throw PosError("Bill ID must be 9 digits: '" + billId + "'");
    }

    // Fixed tokens from the right:
    string orderId = tokens[count - 12];
    Money totalWithoutTax = parseMoney(tokens[count - 11], "Total without tax");
    Money totalTax = parseMoney(tokens[count - 10], "Total tax");
    Money totalDiscount = parseMoney(tokens[count - 9], "Total discount");
    Money totalPayable = parseMoney(tokens[count - 8], "Total payable");
    Money amountReceived = parseMoney(tokens[count - 7], "Amount received");
    Money balanceReturned = parseMoney(tokens[count - 6], "Balance returned");
    string status = tokens[count - 5];
    Money seasonalDiscount = parseMoney(tokens[count - 4], "Seasonal discount");
    Money specialDiscount = parseMoney(tokens[count - 3], "Special discount");
    string billDate = tokens[count - 2];
    string billTime = tokens[count - 1];

    // The customer name is everything between token 0 and count - 12
    string customerName;
    for (int i = 1; i <= count - 13; ++i) {
        if (!customerName.empty()) {
            customerName += " ";
        }
        customerName += tokens[i];
    }
    if (customerName.empty()) {
        customerName = "Walk-in";
    }

    Bill bill;
    bill.id = billId;
    bill.customerName = customerName;
    bill.orderId = orderId;
    bill.totalWithoutTax = totalWithoutTax;
    bill.totalTax = totalTax;
    bill.totalDiscount = totalDiscount;
    bill.totalPayable = totalPayable;
    bill.amountReceived = amountReceived;
    bill.balanceReturned = balanceReturned;
    bill.status = status;
    bill.seasonalDiscount = seasonalDiscount;
    bill.specialDiscount = specialDiscount;
    bill.date = billDate;
    bill.time = billTime;

    return bill;
}

vector<string> BillCodec::format(const Bill& bill) {
    vector<string> tokens;
    tokens.push_back(bill.id);

    // Customer name (words separated into tokens if multiple words)
    string cust = bill.customerName.empty() ? "Walk-in" : bill.customerName;
    // We split customer name by spaces so each word is its own token
    size_t start = 0;
    while (start < cust.size()) {
        while (start < cust.size() && cust[start] == ' ') start++;
        if (start >= cust.size()) break;
        size_t end = start;
        while (end < cust.size() && cust[end] != ' ') end++;
        tokens.push_back(cust.substr(start, end - start));
        start = end;
    }

    tokens.push_back(bill.orderId.empty() ? "-" : bill.orderId);
    tokens.push_back(bill.totalWithoutTax.toString());
    tokens.push_back(bill.totalTax.toString());
    tokens.push_back(bill.totalDiscount.toString());
    tokens.push_back(bill.totalPayable.toString());
    tokens.push_back(bill.amountReceived.toString());
    tokens.push_back(bill.balanceReturned.toString());
    tokens.push_back(bill.status.empty() ? "paid" : bill.status);
    tokens.push_back(bill.seasonalDiscount.toString());
    tokens.push_back(bill.specialDiscount.toString());
    tokens.push_back(bill.date.empty() ? "2026-11-21" : bill.date);
    tokens.push_back(bill.time.empty() ? "00:00:00" : bill.time);

    return tokens;
}

vector<vector<string>> BillCodec::toRecords(const Bill& bill) {
    return { format(bill) };
}

vector<Bill> BillCodec::fromRecords(const vector<vector<string>>& records) {
    vector<Bill> bills;
    for (const auto& rec : records) {
        bills.push_back(parse(rec));
    }
    return bills;
}