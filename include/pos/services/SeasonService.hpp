// SeasonService.hpp                                   Owner: Person 4
// Service managing seasonal discount periods and active date lookups.

#ifndef POS_SEASON_SERVICE_HPP
#define POS_SEASON_SERVICE_HPP

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "pos/domain/SeasonDiscount.hpp"
#include "pos/storage/ISeasonRepository.hpp"

class SeasonService {
public:
    explicit SeasonService(ISeasonRepository& repository);

    std::vector<SeasonDiscount> all() const;
    void add(const std::string& start, const std::string& end, double percent);
    void setActive(std::size_t index, bool flag);

    // Returns discount percent if date is within any active season's [start, end].
    // If multiple active seasons overlap on date, picks the higher percentage.
    std::optional<double> percentFor(const std::string& date) const;

private:
    ISeasonRepository& repository_;
};

#endif // POS_SEASON_SERVICE_HPP
