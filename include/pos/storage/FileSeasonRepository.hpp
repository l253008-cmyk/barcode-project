// FileSeasonRepository.hpp                           Owner: Person 2
// File-backed implementation of ISeasonRepository for Seasons.txt.

#ifndef POS_FILE_SEASON_REPOSITORY_HPP
#define POS_FILE_SEASON_REPOSITORY_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "pos/storage/ISeasonRepository.hpp"

class FileSeasonRepository : public ISeasonRepository {
public:
    explicit FileSeasonRepository(const std::string& path);

    std::vector<SeasonDiscount> all() const override;
    void add(const SeasonDiscount& season) override;
    void setActive(std::size_t index, bool flag) override;

private:
    std::string path_;
};

#endif // POS_FILE_SEASON_REPOSITORY_HPP