// ReceiptFormatter.hpp                                Owner: Person 3
// Formats a Bill into clean, readable receipt text lines (Qt-free).

#ifndef POS_RECEIPT_FORMATTER_HPP
#define POS_RECEIPT_FORMATTER_HPP

#include <string>
#include <vector>

#include "pos/domain/Bill.hpp"

class ReceiptFormatter {
public:
    // Generates formatted text lines for screen display and PDF printing.
    static std::vector<std::string> lines(const Bill& bill, const std::string& shopName = "Store POS System");
};

#endif // POS_RECEIPT_FORMATTER_HPP
