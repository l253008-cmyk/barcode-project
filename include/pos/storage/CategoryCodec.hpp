// CategoryCodec.hpp                                   Owner: Person 2
// Codec for serializing and deserializing ProductCategory records (PCategory.txt).
// Format: <CategoryId> <CategoryName> <CategoryAcronym>#

#ifndef POS_CATEGORY_CODEC_HPP
#define POS_CATEGORY_CODEC_HPP

#include <string>
#include <vector>

#include "pos/domain/ProductCategory.hpp"

class CategoryCodec {
public:
    // Parses a line of tokens into a ProductCategory.
    // Token 0 is categoryId (2 digits), last token is acronym, middle tokens form name.
    static ProductCategory parse(const std::vector<std::string>& tokens);

    // Formats a ProductCategory into tokens for storage.
    static std::vector<std::string> format(const ProductCategory& category);
};

#endif // POS_CATEGORY_CODEC_HPP