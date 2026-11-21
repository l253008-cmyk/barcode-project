// SeasonCodec.cpp                                     Owner: Person 2
// Implementation of SeasonCodec serialization.

#include "pos/storage/SeasonCodec.hpp"
#include "pos/common/DateUtils.hpp"
#include "pos/common/PosError.hpp"

#include <iomanip>
#include <sstream>

using namespace std;

SeasonDiscount SeasonCodec::parse(const vector<string>& tokens) {
    if (tokens.size() != 4) {
        throw PosError("Season line requires exactly 4 tokens, found " + to_string(tokens.size()));
    }

    string start = tokens[0];
    string end = tokens[1];
    if (!pos::DateUtils::isValidDate(start)) {
        throw PosError("Invalid season start date: '" + start + "', expected YYYY-MM-DD");
    }
    if (!pos::DateUtils::isValidDate(end)) {
        throw PosError("Invalid season end date: '" + end + "', expected YYYY-MM-DD");
    }
    if (end < start) {
        throw PosError("Season end date '" + end + "' cannot be before start date '" + start + "'");
    }

    double percent = 0.0;
    try {
        size_t idx = 0;
        percent = stod(tokens[2], &idx);
        if (idx != tokens[2].size()) {
            throw PosError("Invalid discount percent: '" + tokens[2] + "'");
        }
    } catch (...) {
        throw PosError("Invalid discount percent: '" + tokens[2] + "'");
    }

    if (percent <= 0.0 || percent > 100.0) {
        throw PosError("Discount percent must be greater than 0 and at most 100: " + tokens[2]);
    }

    bool active = false;
    if (tokens[3] == "true" || tokens[3] == "1") {
        active = true;
    } else if (tokens[3] == "false" || tokens[3] == "0") {
        active = false;
    } else {
        throw PosError("Invalid season active flag: '" + tokens[3] + "', expected true or false");
    }

    return SeasonDiscount{start, end, percent, active};
}

vector<string> SeasonCodec::format(const SeasonDiscount& season) {
    vector<string> tokens;
    tokens.push_back(season.startDate);
    tokens.push_back(season.endDate);

    ostringstream ss;
    ss << fixed << setprecision(2) << season.percent;
    tokens.push_back(ss.str());

    tokens.push_back(season.active ? "true" : "false");
    return tokens;
}
