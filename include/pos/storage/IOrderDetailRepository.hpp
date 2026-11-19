#ifndef POS_STORAGE_IORDERDETAILREPOSITORY_HPP
#define POS_STORAGE_IORDERDETAILREPOSITORY_HPP

#include <string>

#include "pos/domain/Order.hpp"

namespace pos {

// Where the product lines of orders are kept.
class IOrderDetailRepository {
public:
    virtual ~IOrderDetailRepository() = default;

    virtual void add(const std::string& orderId, const OrderLine& line) = 0;
};

}  // namespace pos

using IOrderDetailRepository = pos::IOrderDetailRepository;

#endif
