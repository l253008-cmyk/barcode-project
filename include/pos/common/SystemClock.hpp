#ifndef POS_COMMON_SYSTEMCLOCK_HPP
#define POS_COMMON_SYSTEMCLOCK_HPP

#include "pos/common/IClock.hpp"

namespace pos {

// The real clock. today() and now() use the computer's local time.
// millis() does not jump when the user changes the system time, so it is safe
// for measuring gaps such as the 250 ms scan interval.
class SystemClock : public IClock {
public:
    std::string today() const override;
    std::string now() const override;
    long long millis() const override;
};

}  // namespace pos

#endif
