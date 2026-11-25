// Bill.hpp                                            
// Represents a bill (Bill.txt) and its item lines (BillDetail.txt).

#ifndef POS_BILL_HPP
#define POS_BILL_HPP

#include "pos/common/Money.hpp"
#include <string>
#include <vector>

// One product line of a bill, as stored in BillDetail.txt.
struct BillDetailLine {
    std::string billId;   // 9 digits, matching Bill.id
    std::string acronym;  // Product acronym
    Money unitPrice;      // Without tax
    double quantity = 0;
    Money discount;       // Total discount on this product line (discountPerUnit * quantity)
};

// Backward compatibility alias
using BillLine = BillDetailLine;

// A whole bill, matching the 14-field master record in Bill.txt.
struct Bill {
    std::string id;             // 9 digits unique ID
    std::string customerName;   // May contain spaces, defaults to "Walk-in"
    std::string orderId;        // 8 digits from Order.txt
    Money totalWithoutTax;      // Sum of unitPrice * quantity
    Money totalTax;             // Sum of tax * quantity
    Money totalDiscount;        // Product discounts only (sum of discount * quantity)
    Money totalPayable;         // Final payable after taxes, seasonal & special discounts
    Money amountReceived;       // Cash/payment given by customer
    Money balanceReturned;      // Change returned (amountReceived - totalPayable)
    std::string status = "paid"; // Order status (e.g. "paid")
    Money seasonalDiscount;     // Seasonal discount in PKR
    Money specialDiscount;      // Special discount in PKR (capped at 8% after seasonal)
    std::string date;           // YYYY-MM-DD
    std::string time;           // HH:MM:SS
    std::vector<BillDetailLine> lines; // Associated detail lines
};

inline bool operator==(const BillDetailLine& a, const BillDetailLine& b) {
    return a.billId == b.billId &&
        a.acronym == b.acronym &&
        a.unitPrice == b.unitPrice &&
        a.quantity == b.quantity &&
        a.discount == b.discount;
}

inline bool operator!=(const BillDetailLine& a, const BillDetailLine& b) {
    return !(a == b);
}

inline bool operator==(const Bill& a, const Bill& b) {
    return a.id == b.id &&
        a.customerName == b.customerName &&
        a.orderId == b.orderId &&
        a.totalWithoutTax == b.totalWithoutTax &&
        a.totalTax == b.totalTax &&
        a.totalDiscount == b.totalDiscount &&
        a.totalPayable == b.totalPayable &&
        a.amountReceived == b.amountReceived &&
        a.balanceReturned == b.balanceReturned &&
        a.status == b.status &&
        a.seasonalDiscount == b.seasonalDiscount &&
        a.specialDiscount == b.specialDiscount &&
        a.date == b.date &&
        a.time == b.time &&
        a.lines == b.lines;
}

inline bool operator!=(const Bill& a, const Bill& b) {
    return !(a == b);
}

#endif // POS_BILL_HPP