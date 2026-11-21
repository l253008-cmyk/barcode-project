#ifndef POS_STORAGE_IORDERDETAILREPOSITORY_HPP
#define POS_STORAGE_IORDERDETAILREPOSITORY_HPP

#include <string>

#include "pos/domain/Order.hpp"

namespace pos {

// Where the product lines of orders are kept.
class IOrderDetailRepository {
public:
    struct Entry {
        std::string orderId;
        OrderLine line;
    };

    virtual ~IOrderDetailRepository() = default;

    virtual void add(const std::string& orderId, const OrderLine& line) = 0;
    virtual std::vector<OrderLine> findByOrderId(const std::string& orderId) const = 0;
    virtual void replaceForOrder(const std::string& orderId, const std::vector<OrderLine>& lines) = 0;
    virtual std::vector<Entry> all() const = 0;
};

}  // namespace pos

using IOrderDetailRepository = pos::IOrderDetailRepository;

#endif
