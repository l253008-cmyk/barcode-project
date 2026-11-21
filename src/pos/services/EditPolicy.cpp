// EditPolicy.cpp                                      Owner: Person 5
// Implementation of EditPolicy rules.

#include "pos/services/EditPolicy.hpp"
#include "pos/common/DateUtils.hpp"
#include "pos/common/PosError.hpp"

using namespace std;

void EditPolicy::check(const Order& order, const string& today, bool managerApproved) {
    if (order.status == OrderStatus::Archived) {
        throw PosError("Archived order '" + order.id + "' cannot be edited");
    }

    if (order.status == OrderStatus::InProgress || order.status == OrderStatus::OnHold) {
        // Freely editable
        return;
    }

    if (order.status == OrderStatus::Paid) {
        if (!managerApproved) {
            throw PosError("Editing paid order '" + order.id + "' requires manager approval");
        }

        if (order.date.empty() || !pos::DateUtils::isValidDate(order.date)) {
            throw PosError("Paid order '" + order.id + "' has invalid payment date: '" + order.date + "'");
        }
        if (!pos::DateUtils::isValidDate(today)) {
            throw PosError("Invalid current date: '" + today + "'");
        }

        int elapsedDays = pos::DateUtils::daysBetween(order.date, today);
        if (elapsedDays < 0) {
            throw PosError("Order payment date is in the future relative to today");
        }
        if (elapsedDays > 7) {
            throw PosError("Paid order '" + order.id + "' was paid " + to_string(elapsedDays) +
                           " days ago and cannot be edited (7-day limit exceeded)");
        }
        return;
    }

    throw PosError("Unknown order status for order '" + order.id + "'");
}
