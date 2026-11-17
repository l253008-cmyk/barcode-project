#ifndef POS_STOCKPOLICY_HPP
#define POS_STOCKPOLICY_HPP

#include "pos/domain/Product.hpp"

// the rule about when we can sell is only written here
class StockPolicy {
public:
    // true if we can sell this quantity of the product
    bool canFulfil(const Product& product, double quantity) const;
};

#endif