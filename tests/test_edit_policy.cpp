// test_edit_policy.cpp                               Owner: Person 5
#include "pos/services/EditPolicy.hpp"
#include "pos/common/PosError.hpp"
#include "test_utils.hpp"
#include <iostream>

using namespace std;

template <typename Func>
static bool checkPasses(const string& name, Func f) {
    try {
        f();
        cout << name << " passed!" << endl;
        return true;
    } catch (const exception& e) {
        cout << name << " failed with exception: " << e.what() << endl;
        return false;
    }
}

int main() {
    bool ok = true;

    // 1. InProgress order: allowed without approval
    Order inProg;
    inProg.id = "00000001";
    inProg.status = OrderStatus::InProgress;
    ok &= checkPasses("InProgress editable", [&]() {
        EditPolicy::check(inProg, "2026-11-21", false);
    });

    // 2. Archived order: always forbidden
    Order arch;
    arch.id = "00000002";
    arch.status = OrderStatus::Archived;
    ok &= checkThrowsAs<PosError>("Archived forbidden", [&]() {
        EditPolicy::check(arch, "2026-11-21", true);
    });

    // 3. Paid order without approval: throws
    Order paid;
    paid.id = "00000003";
    paid.status = OrderStatus::Paid;
    paid.date = "2026-11-18";
    ok &= checkThrowsAs<PosError>("Paid without approval throws", [&]() {
        EditPolicy::check(paid, "2026-11-21", false);
    });

    // 4. Paid order within 7 days (3 days ago) with approval: passes
    ok &= checkPasses("Paid 3 days ago with approval passes", [&]() {
        EditPolicy::check(paid, "2026-11-21", true);
    });

    // 5. Paid order exactly 7 days ago with approval: passes
    paid.date = "2026-11-14";
    ok &= checkPasses("Paid 7 days ago with approval passes", [&]() {
        EditPolicy::check(paid, "2026-11-21", true);
    });

    // 6. Paid order 8 days ago with approval: throws!
    paid.date = "2026-11-13";
    ok &= checkThrowsAs<PosError>("Paid 8 days ago throws (exceeds 7-day limit)", [&]() {
        EditPolicy::check(paid, "2026-11-21", true);
    });

    if (ok) {
        cout << "test_edit_policy: all passed\n";
        return 0;
    }
    cout << "test_edit_policy: some tests failed\n";
    return 1;
}
