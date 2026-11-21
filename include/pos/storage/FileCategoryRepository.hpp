// FileCategoryRepository.hpp                         Owner: Person 2
// File-backed implementation of ICategoryRepository for PCategory.txt.

#ifndef POS_FILE_CATEGORY_REPOSITORY_HPP
#define POS_FILE_CATEGORY_REPOSITORY_HPP

#include <string>
#include <vector>

#include "pos/storage/ICategoryRepository.hpp"

class FileCategoryRepository : public ICategoryRepository {
public:
    explicit FileCategoryRepository(const std::string& path);

    std::vector<ProductCategory> all() const override;
    std::optional<ProductCategory> findById(const std::string& id) const override;
    void add(const ProductCategory& category) override;

private:
    std::string path_;
};

#endif // POS_FILE_CATEGORY_REPOSITORY_HPP
