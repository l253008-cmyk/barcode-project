#ifndef POS_COMMON_ICLOCK_HPP
#define POS_COMMON_ICLOCK_HPP

#include <string>

namespace pos {

// Everything that needs the time asks this interface, so tests can hand in a
// clock that always says the same thing.
class IClock {
public:
    virtual ~IClock() = default;

    virtual std::string today() const = 0;   // YYYY-MM-DD
    virtual std::string now() const = 0;     // HH:MM:SS
    virtual long long millis() const = 0;    // steady milliseconds, for time gaps only
};

}  // namespace pos

#endif
