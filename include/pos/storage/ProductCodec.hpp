#ifndef POS_PRODUCTCODEC_HPP
#define POS_PRODUCTCODEC_HPP

#include <string>
#include <vector>
#include "pos/domain/Product.hpp"

// only knows how one line of Product.txt looks
class ProductCodec {
public:
    // fields of one line (the # is already removed) to a Product
    // error if the line is not a proper product
    static Product parse(const std::vector<std::string>& tokens);

    // a Product back to the fields of one line
    static std::vector<std::string> format(const Product& product);
};

#endif