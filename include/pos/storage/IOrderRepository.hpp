#ifndef POS_STORAGE_IORDERREPOSITORY_HPP
#define POS_STORAGE_IORDERREPOSITORY_HPP

#include <optional>
#include <string>
#include <vector>

#include "pos/domain/Order.hpp"

namespace pos {

    // Where orders are kept. Only the order header is stored here; the product
    // lines go through IOrderDetailRepository.
    class IOrderRepository {
    public:
        virtual ~IOrderRepository() = default;

        virtual void add(const Order& order) = 0;
        virtual std::optional<Order> findById(const std::string& id) const = 0;
        virtual std::vector<Order> all() const = 0;
        virtual void update(const Order& order) = 0;

        // Id of the most recently added order, or an empty string when there is none.
        virtual std::string lastId() const = 0;
    };

}  // namespace pos

using IOrderRepository = pos::IOrderRepository;

#endif