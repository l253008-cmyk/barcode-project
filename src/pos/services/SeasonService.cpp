// SeasonService.cpp                                   Owner: Person 4
// Implementation of SeasonService.

#include "pos/services/SeasonService.hpp"
#include "pos/common/DateUtils.hpp"
#include "pos/common/PosError.hpp"

using namespace std;

SeasonService::SeasonService(ISeasonRepository& repository)
    : repository_(repository) {
}

vector<SeasonDiscount> SeasonService::all() const {
    return repository_.all();
}

void SeasonService::add(const string& start, const string& end, double percent) {
    if (!pos::DateUtils::isValidDate(start)) {
        throw PosError("Invalid start date: '" + start + "', expected YYYY-MM-DD");
    }
    if (!pos::DateUtils::isValidDate(end)) {
        throw PosError("Invalid end date: '" + end + "', expected YYYY-MM-DD");
    }
    if (end < start) {
        throw PosError("End date '" + end + "' cannot be before start date '" + start + "'");
    }
    if (percent <= 0.0 || percent > 100.0) {
        throw PosError("Discount percent must be greater than 0 and at most 100%");
    }

    SeasonDiscount s{ start, end, percent, true };
    repository_.add(s);
}

void SeasonService::setActive(size_t index, bool flag) {
    repository_.setActive(index, flag);
}

optional<double> SeasonService::percentFor(const string& date) const {
    if (!pos::DateUtils::isValidDate(date)) {
        return nullopt;
    }

    vector<SeasonDiscount> seasons = repository_.all();
    optional<double> highestPercent;

    for (const auto& s : seasons) {
        if (s.active && pos::DateUtils::isDateInRange(date, s.startDate, s.endDate)) {
            if (!highestPercent.has_value() || s.percent > highestPercent.value()) {
                highestPercent = s.percent;
            }
        }
    }

    return highestPercent;
}