// EditPolicy.hpp                                      Owner: Person 5
// Enforces rules for opening and editing existing orders.

#ifndef POS_EDIT_POLICY_HPP
#define POS_EDIT_POLICY_HPP

#include <string>
#include "pos/domain/Order.hpp"

class EditPolicy {
public:
    // Throws PosError if editing the order is not permitted.
    // - InProgress or OnHold: permitted without manager approval.
    // - Paid: permitted ONLY if managerApproved is true AND daysBetween(order.date, today) <= 7.
    // - Archived: never permitted.
    static void check(const Order& order, const std::string& today, bool managerApproved);
};

#endif // POS_EDIT_POLICY_HPP
