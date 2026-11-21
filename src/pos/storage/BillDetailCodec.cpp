// BillDetailCodec.cpp                                 Owner: Person 2
// Implementation of BillDetailCodec serialization.

#include "pos/storage/BillDetailCodec.hpp"
#include "pos/common/PosError.hpp"

using namespace std;

BillDetailLine BillDetailCodec::parse(const vector<string>& tokens) {
    if (tokens.size() != 5) {
        throw PosError("BillDetail line requires exactly 5 tokens, found " + to_string(tokens.size()));
    }

    string billId = tokens[0];
    if (billId.size() != 9) {
        throw PosError("Bill ID in BillDetail must be 9 digits: '" + billId + "'");
    }

    string acronym = tokens[1];
    if (acronym.empty() || acronym.size() > 8) {
        throw PosError("Product acronym in BillDetail must be between 1 and 8 characters: '" + acronym + "'");
    }

    Money unitPrice;
    try {
        unitPrice = Money::parse(tokens[2]);
    } catch (const PosError&) {
        throw PosError("Invalid unit price in BillDetail: '" + tokens[2] + "'");
    }
    if (unitPrice.paisa() < 0) {
        throw PosError("Unit price in BillDetail cannot be negative: '" + tokens[2] + "'");
    }

    double quantity = 0.0;
    try {
        size_t idx = 0;
        quantity = stod(tokens[3], &idx);
        if (idx != tokens[3].size()) {
            throw PosError("Invalid quantity in BillDetail: '" + tokens[3] + "'");
        }
    } catch (...) {
        throw PosError("Invalid quantity in BillDetail: '" + tokens[3] + "'");
    }
    if (!(quantity > 0.0)) {
        throw PosError("Quantity in BillDetail must be greater than zero: '" + tokens[3] + "'");
    }

    Money discount;
    try {
        discount = Money::parse(tokens[4]);
    } catch (const PosError&) {
        throw PosError("Invalid discount in BillDetail: '" + tokens[4] + "'");
    }
    if (discount.paisa() < 0) {
        throw PosError("Discount in BillDetail cannot be negative: '" + tokens[4] + "'");
    }

    return BillDetailLine{billId, acronym, unitPrice, quantity, discount};
}

vector<string> BillDetailCodec::format(const BillDetailLine& line) {
    vector<string> tokens;
    tokens.push_back(line.billId);
    tokens.push_back(line.acronym);
    tokens.push_back(line.unitPrice.toString());

    // Format quantity cleanly (strip unnecessary trailing zeros)
    string qStr = to_string(line.quantity);
    if (qStr.find('.') != string::npos) {
        while (!qStr.empty() && qStr.back() == '0') {
            qStr.pop_back();
        }
        if (!qStr.empty() && qStr.back() == '.') {
            qStr.pop_back();
        }
    }
    tokens.push_back(qStr.empty() ? "0" : qStr);

    tokens.push_back(line.discount.toString());
    return tokens;
}
