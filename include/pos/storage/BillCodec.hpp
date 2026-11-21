#ifndef POS_BILL_CODEC_HPP
#define POS_BILL_CODEC_HPP

#include "pos/domain/Bill.hpp"
#include <string>
#include <vector>

// Knows only the format of Bill.txt. It does not read or write the file
// (the repository does that) and it does not calculate anything.
//
// One record (list of tokens) is one line of the file, without the # marker:
//   billId  customer name words...  orderId  acronym  unitPrice  discount
//   quantity  totalWithoutTax  totalTax  totalDiscount  totalPayable  status
// The first token and the last 10 tokens are fixed. Everything between them
// is the customer name.
class BillCodec {
public:
    // One record for every line of the bill. All records of one bill carry
    // the same bill id and the same totals.
    static std::vector<std::vector<std::string>> toRecords(const Bill& bill);

    // Groups the records that share a bill id into bills.
    // Throws PosError for a bad record.
    static std::vector<Bill> fromRecords(const std::vector<std::vector<std::string>>& records);
};

#endif // POS_BILL_CODEC_HPP