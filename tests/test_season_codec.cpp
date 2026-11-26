// test_season_codec.cpp                              Owner: Person 2
#include "pos/storage/SeasonCodec.hpp"
#include "pos/common/PosError.hpp"
#include "test_utils.hpp"
#include <iostream>

using namespace std;

int main() {
    bool ok = true;

    // 1. Format and parse valid season
    SeasonDiscount season{ "2026-11-01", "2026-11-30", 10.0, true };
    auto tokens = SeasonCodec::format(season);
    ok &= checkResult("Token count", "4", to_string(tokens.size()));
    ok &= checkResult("Start date", "2026-11-01", tokens[0]);
    ok &= checkResult("End date", "2026-11-30", tokens[1]);
    ok &= checkResult("Percent", "10.00", tokens[2]);
    ok &= checkResult("Active", "true", tokens[3]);

    auto parsed = SeasonCodec::parse(tokens);
    ok &= checkResult("Parsed start", season.startDate, parsed.startDate);
    ok &= checkResult("Parsed end", season.endDate, parsed.endDate);
    ok &= checkTrue("Parsed percent", parsed.percent == 10.0);
    ok &= checkTrue("Parsed active", parsed.active == true);

    // 2. Reject start > end
    ok &= checkThrowsAs<PosError>("Reject inverted dates", [&]() {
        SeasonCodec::parse({ "2026-12-01", "2026-11-01", "10.0", "true" });
        });

    // 3. Reject negative or >100 percent
    ok &= checkThrowsAs<PosError>("Reject negative percent", [&]() {
        SeasonCodec::parse({ "2026-11-01", "2026-11-30", "-5.0", "true" });
        });
    ok &= checkThrowsAs<PosError>("Reject >100 percent", [&]() {
        SeasonCodec::parse({ "2026-11-01", "2026-11-30", "150.0", "true" });
        });

    if (ok) {
        cout << "test_season_codec: all passed\n";
        return 0;
    }
    cout << "test_season_codec: some tests failed\n";
    return 1;
}