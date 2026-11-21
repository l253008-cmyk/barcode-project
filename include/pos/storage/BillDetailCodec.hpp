// BillDetailCodec.hpp                                 Owner: Person 2
// Codec for serializing and deserializing BillDetailLine records (BillDetail.txt).
// Format: <BillId> <ProductAcronym> <UnitPriceWithoutTax> <Quantity> <DiscountOnProduct>#

#ifndef POS_BILL_DETAIL_CODEC_HPP
#define POS_BILL_DETAIL_CODEC_HPP

#include <string>
#include <vector>

#include "pos/domain/Bill.hpp"

class BillDetailCodec {
public:
    // Parses a 5-token line into a BillDetailLine.
    static BillDetailLine parse(const std::vector<std::string>& tokens);

    // Formats a BillDetailLine into 5 tokens for storage.
    static std::vector<std::string> format(const BillDetailLine& line);
};

#endif // POS_BILL_DETAIL_CODEC_HPP
