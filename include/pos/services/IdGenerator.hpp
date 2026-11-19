#ifndef POS_SERVICES_IDGENERATOR_HPP
#define POS_SERVICES_IDGENERATOR_HPP

#include <string>

#include "pos/common/IClock.hpp"
#include "pos/storage/IBillRepository.hpp"
#include "pos/storage/IOrderRepository.hpp"

namespace pos {

// Makes the next order id and bill id by looking at the last one stored.
//   Order id: 6-digit running number + 2-digit day of the month,
//             e.g. 00012314 is order 123 made on the 14th.
//   Bill id:  9-digit running number, e.g. 000000045.
// Both start at 1 when the file is empty. The numbers keep growing across
// days and months; only the last two digits of an order id follow the date.
// An id is not reserved: asking twice before saving gives the same id twice.
class IdGenerator {
public:
    IdGenerator(IOrderRepository& orders, IBillRepository& bills, IClock& clock);

    std::string nextOrderId();
    std::string nextBillId();

private:
    IOrderRepository& orders_;
    IBillRepository& bills_;
    IClock& clock_;
};

}  // namespace pos

using IdGenerator = pos::IdGenerator;

#endif
