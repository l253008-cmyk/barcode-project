// CategoryService.hpp                                 Owner: Person 5
// Service managing product categories and validation (PCategory.txt).

#ifndef POS_CATEGORY_SERVICE_HPP
#define POS_CATEGORY_SERVICE_HPP

#include <optional>
#include <string>
#include <vector>

#include "pos/domain/ProductCategory.hpp"
#include "pos/storage/ICategoryRepository.hpp"

class CategoryService {
public:
    explicit CategoryService(ICategoryRepository& repository);

    std::vector<ProductCategory> all() const;
    void add(const std::string& id, const std::string& name, const std::string& acronym);
    std::optional<ProductCategory> findById(const std::string& id) const;

private:
    ICategoryRepository& repository_;
};

#endif // POS_CATEGORY_SERVICE_HPP
