// ISeasonRepository.hpp                               Owner: Person 2
// Interface for season discounts storage (Seasons.txt).

#ifndef POS_ISEASON_REPOSITORY_HPP
#define POS_ISEASON_REPOSITORY_HPP

#include <cstddef>
#include <vector>

#include "pos/domain/SeasonDiscount.hpp"

class ISeasonRepository {
public:
    virtual ~ISeasonRepository() = default;

    virtual std::vector<SeasonDiscount> all() const = 0;
    virtual void add(const SeasonDiscount& season) = 0;
    virtual void setActive(std::size_t index, bool flag) = 0;
};

#endif // POS_ISEASON_REPOSITORY_HPP
