// test_category_service.cpp                          Owner: Person 5
#include "pos/services/CategoryService.hpp"
#include "pos/storage/ICategoryRepository.hpp"
#include "pos/common/PosError.hpp"
#include "test_utils.hpp"
#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

class FakeCategoryRepo : public ICategoryRepository {
public:
    vector<ProductCategory> cats;

    vector<ProductCategory> all() const override { return cats; }

    optional<ProductCategory> findById(const string& id) const override {
        for (const auto& c : cats) {
            if (c.id == id) return c;
        }
        return nullopt;
    }

    void add(const ProductCategory& cat) override {
        cats.push_back(cat);
    }
};

int main() {
    bool ok = true;

    FakeCategoryRepo repo;
    CategoryService service(repo);

    // 1. Add valid categories
    service.add("10", "Groceries", "GRO");
    service.add("20", "Beverages", "BEV");

    ok &= checkResult("Count is 2", "2", to_string(service.all().size()));
    ok &= checkTrue("Find by ID 10", service.findById("10").has_value());
    ok &= checkTrue("Find by ID 20", service.findById("20").has_value());
    ok &= checkTrue("Find by ID 30", !service.findById("30").has_value());

    // 2. Reject duplicate ID
    ok &= checkThrowsAs<PosError>("Duplicate ID throws", [&]() {
        service.add("10", "Other", "OTH");
    });

    // 3. Reject duplicate Acronym
    ok &= checkThrowsAs<PosError>("Duplicate Acronym throws", [&]() {
        service.add("30", "Beverages Two", "BEV");
    });

    // 4. Reject invalid ID (not 10-99)
    ok &= checkThrowsAs<PosError>("ID < 10 throws", [&]() {
        service.add("09", "Invalid", "INV");
    });

    if (ok) {
        cout << "test_category_service: all passed\n";
        return 0;
    }
    cout << "test_category_service: some tests failed\n";
    return 1;
}
