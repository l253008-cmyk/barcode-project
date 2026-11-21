// CategoryCodec.cpp                                   Owner: Person 2
// Implementation of CategoryCodec serialization.

#include "pos/storage/CategoryCodec.hpp"
#include "pos/common/PosError.hpp"

using namespace std;

ProductCategory CategoryCodec::parse(const vector<string>& tokens) {
    if (tokens.size() < 3) {
        throw PosError("Category line requires at least 3 tokens (id, name, acronym)");
    }

    string id = tokens[0];
    if (id.size() != 2 || id[0] < '1' || id[0] > '9' || id[1] < '0' || id[1] > '9') {
        throw PosError("Category ID must be 2 digits (10 to 99): '" + id + "'");
    }

    string acronym = tokens.back();
    if (acronym.empty() || acronym.size() > 8) {
        throw PosError("Category acronym must be between 1 and 8 characters: '" + acronym + "'");
    }

    // Middle tokens form the category name
    string name;
    for (size_t i = 1; i + 1 < tokens.size(); ++i) {
        if (!name.empty()) {
            name += " ";
        }
        name += tokens[i];
    }

    if (name.empty()) {
        throw PosError("Category name cannot be empty");
    }

    return ProductCategory{id, name, acronym};
}

vector<string> CategoryCodec::format(const ProductCategory& category) {
    vector<string> tokens;
    tokens.push_back(category.id);
    tokens.push_back(category.name);
    tokens.push_back(category.acronym);
    return tokens;
}
