#ifndef POS_BILL_HPP
#define POS_BILL_HPP

#include "pos/common/Money.hpp"
#include <string>
#include <vector>

// One product line of a bill, as it is written in Bill.txt.
struct BillLine {
    std::string acronym;
    Money unitPrice;      // without tax
    Money discount;       // per unit
    double quantity = 0;
};

// A whole bill. The four totals belong to the whole bill, not to one line.
struct Bill {
    std::string id;             // 9 digits
    std::string customerName;   // may contain spaces, empty when not given
    std::string orderId;        // 8 digits, from Order.txt
    std::vector<BillLine> lines;
    Money totalWithoutTax;
    Money totalTax;
    Money totalDiscount;
    Money totalPayable;
    std::string status;         // the order status text, for example "paid"
};

// Equality is used by the tests (a bill written and read again must be equal).
bool operator==(const BillLine& a, const BillLine& b);
bool operator!=(const BillLine& a, const BillLine& b);
bool operator==(const Bill& a, const Bill& b);
bool operator!=(const Bill& a, const Bill& b);

#endif // POS_BILL_HPP