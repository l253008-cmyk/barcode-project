#ifndef POS_I_BILL_REPOSITORY_HPP
#define POS_I_BILL_REPOSITORY_HPP

#include <optional>
#include <string>
#include <vector>

#include "pos/domain/Bill.hpp"

// Where bills are stored. Services use this interface only, so the text file
// can be replaced by a database without changing any service.
class IBillRepository {
public:
    virtual ~IBillRepository() = default;

    // Saves the whole bill.
    virtual void add(const Bill& bill) = 0;

    // Replaces an existing bill by bill.id (both master line and detail lines).
    virtual void replace(const Bill& bill) = 0;

    // Finds bill by associated orderId.
    virtual std::optional<Bill> findByOrderId(const std::string& orderId) const = 0;

    // Returns all bills with their lines loaded.
    virtual std::vector<Bill> all() const = 0;

    // Id of the last saved bill, or "" when there is no bill yet.
    virtual std::string lastId() const = 0;
};

#endif // POS_I_BILL_REPOSITORY_HPP