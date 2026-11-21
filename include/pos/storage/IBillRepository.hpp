#ifndef POS_I_BILL_REPOSITORY_HPP
#define POS_I_BILL_REPOSITORY_HPP

#include "pos/domain/Bill.hpp"
#include <string>

// Where bills are stored. Services use this interface only, so the text file
// can be replaced by a database without changing any service.
class IBillRepository {
public:
    virtual ~IBillRepository() = default;

    // Saves the whole bill.
    virtual void add(const Bill& bill) = 0;

    // Id of the last saved bill, or "" when there is no bill yet.
    virtual std::string lastId() const = 0;
};

#endif // POS_I_BILL_REPOSITORY_HPP