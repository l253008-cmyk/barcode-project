#ifndef POS_STORAGE_ORDERCODEC_HPP
#define POS_STORAGE_ORDERCODEC_HPP

#include <string>

#include "pos/common/List.hpp"
#include "pos/domain/Order.hpp"

namespace pos {

// Knows the layout of one line of Order.txt and nothing else:
//   id date time <name...> contact status payment mode merchant#
// The first 3 and the last 4 tokens are fixed. In the middle, the last token
// is the contact and whatever comes before it is the customer name.
// An empty name, contact or merchant is written as "-".
// The order lines are not part of Order.txt, so parse() returns lines empty.
class OrderCodec {
public:
    // From the tokens TextRecordFile gives back.
    static Order parse(const List<std::string>& tokens);

    // From a whole line, with or without the final '#'.
    static Order parseLine(const std::string& line);

    // The tokens to hand to TextRecordFile::append.
    static List<std::string> toTokens(const Order& order);

    // The finished line as it appears in the file, '#' included.
    static std::string format(const Order& order);
};

}  // namespace pos

#endif
