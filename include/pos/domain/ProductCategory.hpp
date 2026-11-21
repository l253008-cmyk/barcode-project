// ProductCategory.hpp                                 Owner: Person 3
// Represents a product category (PCategory.txt).
// Category ID is 2 digits (10 to 99), which prefixes product barcodes.

#ifndef POS_PRODUCT_CATEGORY_HPP
#define POS_PRODUCT_CATEGORY_HPP

#include <string>

struct ProductCategory {
    std::string id;       // 2 digits: "10" to "99"
    std::string name;     // Category name (may contain spaces)
    std::string acronym;  // Up to 8 chars, uppercase letters/digits
};

inline bool operator==(const ProductCategory& a, const ProductCategory& b) {
    return a.id == b.id && a.name == b.name && a.acronym == b.acronym;
}

inline bool operator!=(const ProductCategory& a, const ProductCategory& b) {
    return !(a == b);
}

#endif // POS_PRODUCT_CATEGORY_HPP
