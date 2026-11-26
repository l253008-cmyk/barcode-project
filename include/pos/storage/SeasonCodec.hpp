// SeasonCodec.hpp                                     Owner: Person 2
// Codec for serializing and deserializing SeasonDiscount records (Seasons.txt).
// Format: <StartDate> <EndDate> <DiscountPercent> <isActive>#

#ifndef POS_SEASON_CODEC_HPP
#define POS_SEASON_CODEC_HPP

#include <string>
#include <vector>

#include "pos/domain/SeasonDiscount.hpp"

class SeasonCodec {
public:
    // Parses a 4-token line into SeasonDiscount.
    static SeasonDiscount parse(const std::vector<std::string>& tokens);

    // Formats a SeasonDiscount into 4 tokens for storage.
    static std::vector<std::string> format(const SeasonDiscount& season);
};

#endif // POS_SEASON_CODEC_HPP