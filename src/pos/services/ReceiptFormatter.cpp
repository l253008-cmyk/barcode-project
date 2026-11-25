// ReceiptFormatter.cpp                              Owner: Person 3
// Formats a Bill into clean, readable receipt text lines (Qt-free).

#include "pos/services/ReceiptFormatter.hpp"

#include <iomanip>
#include <sstream>

std::vector<std::string> ReceiptFormatter::lines(const Bill& bill, const std::string& shopName) {
    std::vector<std::string> out;
    const std::string sep = "------------------------------------------";

    out.push_back("==========================================");
    out.push_back("          " + shopName);
    out.push_back("==========================================");
    out.push_back("Bill ID:  " + bill.id);
    out.push_back("Order ID: " + bill.orderId);
    out.push_back("Customer: " + bill.customerName);
    out.push_back("Date:     " + bill.date + " " + bill.time);
    out.push_back("Status:   " + bill.status);
    out.push_back(sep);

    std::ostringstream header;
    header << std::left << std::setw(12) << "ITEM"
        << std::right << std::setw(6) << "QTY"
        << std::setw(10) << "PRICE"
        << std::setw(8) << "DISC"
        << std::setw(10) << "TOTAL";
    out.push_back(header.str());
    out.push_back(sep);

    for (const auto& item : bill.lines) {
        std::ostringstream line;
        Money lineTotal = (item.unitPrice * item.quantity) - item.discount;
        std::ostringstream qtyStr;
        if (item.quantity == static_cast<long long>(item.quantity)) {
            qtyStr << static_cast<long long>(item.quantity);
        }
        else {
            qtyStr << std::fixed << std::setprecision(2) << item.quantity;
        }

        line << std::left << std::setw(12) << item.acronym.substr(0, 11)
            << std::right << std::setw(6) << qtyStr.str()
            << std::setw(10) << item.unitPrice.toString()
            << std::setw(8) << item.discount.toString()
            << std::setw(10) << lineTotal.toString();
        out.push_back(line.str());
    }

    out.push_back(sep);
    out.push_back("Total without Tax:   " + bill.totalWithoutTax.toString());
    out.push_back("Total Tax:           " + bill.totalTax.toString());
    out.push_back("Product Discount:   -" + bill.totalDiscount.toString());

    if (bill.seasonalDiscount.paisa() > 0) {
        out.push_back("Seasonal Discount:  -" + bill.seasonalDiscount.toString());
    }
    if (bill.specialDiscount.paisa() > 0) {
        out.push_back("Special Discount:   -" + bill.specialDiscount.toString());
    }

    out.push_back(sep);
    out.push_back("TOTAL PAYABLE:       " + bill.totalPayable.toString());
    out.push_back("Amount Received:     " + bill.amountReceived.toString());
    out.push_back("Balance Returned:    " + bill.balanceReturned.toString());
    out.push_back("==========================================");
    out.push_back("          THANK YOU FOR YOUR VISIT!       ");
    out.push_back("==========================================");

    return out;
}