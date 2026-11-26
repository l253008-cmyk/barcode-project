// CategoryService.cpp                                 Owner: Person 5
// Implementation of CategoryService.

#include "pos/services/CategoryService.hpp"
#include "pos/common/PosError.hpp"

using namespace std;

CategoryService::CategoryService(ICategoryRepository& repository)
    : repository_(repository) {
}

vector<ProductCategory> CategoryService::all() const {
    return repository_.all();
}

void CategoryService::add(const string& id, const string& name, const string& acronym) {
    if (id.size() != 2 || id[0] < '1' || id[0] > '9' || id[1] < '0' || id[1] > '9') {
        throw PosError("Category ID must be exactly 2 digits between 10 and 99: '" + id + "'");
    }
    if (name.empty()) {
        throw PosError("Category name cannot be empty");
    }
    if (acronym.empty() || acronym.size() > 8) {
        throw PosError("Category acronym must be between 1 and 8 characters: '" + acronym + "'");
    }

    // Check uniqueness
    vector<ProductCategory> existing = all();
    for (const auto& cat : existing) {
        if (cat.id == id) {
            throw PosError("Category ID '" + id + "' is already in use by '" + cat.name + "'");
        }
        if (cat.acronym == acronym) {
            throw PosError("Category acronym '" + acronym + "' is already in use by '" + cat.name + "'");
        }
    }

    ProductCategory cat{id, name, acronym};
    repository_.add(cat);
}

optional<ProductCategory> CategoryService::findById(const string& id) const {
    return repository_.findById(id);
}
