// SeasonDiscount.hpp                                  Owner: Person 3
// Represents a seasonal discount configuration (Seasons.txt).

#ifndef POS_SEASON_DISCOUNT_HPP
#define POS_SEASON_DISCOUNT_HPP

#include <string>

struct SeasonDiscount {
    std::string startDate;  // YYYY-MM-DD (inclusive)
    std::string endDate;    // YYYY-MM-DD (inclusive)
    double percent = 0.0;   // e.g. 10.0 for 10%
    bool active = false;    // true if currently active
};

inline bool operator==(const SeasonDiscount& a, const SeasonDiscount& b) {
    return a.startDate == b.startDate && a.endDate == b.endDate &&
           a.percent == b.percent && a.active == b.active;
}

inline bool operator!=(const SeasonDiscount& a, const SeasonDiscount& b) {
    return !(a == b);
}

#endif // POS_SEASON_DISCOUNT_HPP
