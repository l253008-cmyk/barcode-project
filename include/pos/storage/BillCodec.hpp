// BillCodec.hpp                                       Owner: Person 2
// Codec for serializing and deserializing Bill summary records (Bill.txt).
// 14 fields:
// billId customerName orderId totalWithoutTax totalTax totalDiscount
// totalPayable amountReceived balanceReturned status seasonalDiscount
// specialDiscount billDate billTime

#ifndef POS_BILL_CODEC_HPP
#define POS_BILL_CODEC_HPP

#include "pos/domain/Bill.hpp"
#include <string>
#include <vector>

class BillCodec {
public:
    // Parses a single 14+ token record into a Bill summary.
    static Bill parse(const std::vector<std::string>& tokens);

    // Formats a Bill summary into a single line of tokens for Bill.txt.
    static std::vector<std::string> format(const Bill& bill);

    // Convenience / compatibility methods
    static std::vector<std::vector<std::string>> toRecords(const Bill& bill);
    static std::vector<Bill> fromRecords(const std::vector<std::vector<std::string>>& records);
};

#endif // POS_BILL_CODEC_HPP