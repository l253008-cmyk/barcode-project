#ifndef POS_STORAGE_ORDERDETAILCODEC_HPP
#define POS_STORAGE_ORDERDETAILCODEC_HPP

#include <string>

#include "pos/common/List.hpp"
#include "pos/domain/Order.hpp"

namespace pos {

// One row of OrderDetail.txt as read back from the file.
struct OrderDetailRecord {
    std::string orderId;
    int productId = 0;
    std::string barcode;
    double quantity = 0.0;
};

// Knows the layout of one line of OrderDetail.txt:
//   orderId productId barcode quantity#
class OrderDetailCodec {
public:
    static OrderDetailRecord parse(const List<std::string>& tokens);
    static OrderDetailRecord parseLine(const std::string& line);

    static List<std::string> toTokens(const std::string& orderId, const OrderLine& line);
    static std::string format(const std::string& orderId, const OrderLine& line);
};

}  // namespace pos

#endif
