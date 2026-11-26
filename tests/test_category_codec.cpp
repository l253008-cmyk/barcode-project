// test_category_codec.cpp                            Owner: Person 2
#include "pos/storage/CategoryCodec.hpp"
#include "pos/common/PosError.hpp"
#include "test_utils.hpp"
#include <iostream>

using namespace std;

int main() {
    bool ok = true;

    // 1. Format and parse valid category
    ProductCategory cat{ "10", "Beverages", "BEV" };
    auto tokens = CategoryCodec::format(cat);
    ok &= checkResult("Token count", "3", to_string(tokens.size()));
    ok &= checkResult("Token 0 ID", "10", tokens[0]);
    ok &= checkResult("Token 1 Name", "Beverages", tokens[1]);
    ok &= checkResult("Token 2 Acronym", "BEV", tokens[2]);

    auto parsed = CategoryCodec::parse(tokens);
    ok &= checkResult("Parsed ID", cat.id, parsed.id);
    ok &= checkResult("Parsed Name", cat.name, parsed.name);
    ok &= checkResult("Parsed Acronym", cat.acronym, parsed.acronym);

    // 2. Reject ID with leading zero (01..09)
    ok &= checkThrowsAs<PosError>("Reject ID with leading zero", [&]() {
        CategoryCodec::parse({ "05", "Snacks", "SNK" });
        });

    // 3. Reject non-2-digit IDs
    ok &= checkThrowsAs<PosError>("Reject 1-digit ID", [&]() {
        CategoryCodec::parse({ "5", "Snacks", "SNK" });
        });
    ok &= checkThrowsAs<PosError>("Reject 3-digit ID", [&]() {
        CategoryCodec::parse({ "100", "Snacks", "SNK" });
        });

    // 4. Multi-word category name
    ProductCategory multi{ "25", "Fresh Fruits and Vegetables", "FFV" };
    auto multiTokens = CategoryCodec::format(multi);
    auto multiParsed = CategoryCodec::parse(multiTokens);
    ok &= checkResult("Multi-word name", multi.name, multiParsed.name);
    ok &= checkResult("Multi-word acronym", multi.acronym, multiParsed.acronym);

    if (ok) {
        cout << "test_category_codec: all passed\n";
        return 0;
    }
    cout << "test_category_codec: some tests failed\n";
    return 1;
}