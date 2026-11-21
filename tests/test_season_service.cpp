// test_season_service.cpp                            Owner: Person 4
#include "pos/services/SeasonService.hpp"
#include "pos/storage/ISeasonRepository.hpp"
#include "pos/common/PosError.hpp"
#include "test_utils.hpp"
#include <iostream>
#include <vector>

using namespace std;

class FakeSeasonRepo : public ISeasonRepository {
public:
    vector<SeasonDiscount> list;

    vector<SeasonDiscount> all() const override { return list; }

    void add(const SeasonDiscount& s) override {
        list.push_back(s);
    }

    void setActive(size_t index, bool activeFlag) override {
        if (index < list.size()) {
            list[index].active = activeFlag;
        }
    }
};

int main() {
    bool ok = true;

    FakeSeasonRepo repo;
    SeasonService service(repo);

    // Add seasons
    service.add("2026-11-01", "2026-11-30", 5.0);
    service.add("2026-11-15", "2026-11-20", 12.0); // Overlapping, higher percent
    service.add("2026-12-01", "2026-12-31", 15.0);

    // 1. Date before any season
    auto p1 = service.percentFor("2026-10-31");
    ok &= checkTrue("Before seasons has no discount", !p1.has_value());

    // 2. Date in 5% season
    auto p2 = service.percentFor("2026-11-05");
    ok &= checkTrue("Has discount", p2.has_value());
    if (p2) ok &= checkTrue("5% discount", *p2 == 5.0);

    // 3. Overlapping date picks highest percent (12%)
    auto p3 = service.percentFor("2026-11-18");
    ok &= checkTrue("Has discount", p3.has_value());
    if (p3) ok &= checkTrue("Picks higher 12% discount", *p3 == 12.0);

    // 4. Deactivate 12% season -> now returns 5%
    service.setActive(1, false);
    auto p4 = service.percentFor("2026-11-18");
    ok &= checkTrue("Has discount", p4.has_value());
    if (p4) ok &= checkTrue("Now falls back to 5%", *p4 == 5.0);

    if (ok) {
        cout << "test_season_service: all passed\n";
        return 0;
    }
    cout << "test_season_service: some tests failed\n";
    return 1;
}
