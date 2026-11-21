// ICategoryRepository.hpp                             Owner: Person 2
// Interface for product category storage (PCategory.txt).

#ifndef POS_ICATEGORY_REPOSITORY_HPP
#define POS_ICATEGORY_REPOSITORY_HPP

#include <optional>
#include <string>
#include <vector>

#include "pos/domain/ProductCategory.hpp"

class ICategoryRepository {
public:
    virtual ~ICategoryRepository() = default;

    virtual std::vector<ProductCategory> all() const = 0;
    virtual std::optional<ProductCategory> findById(const std::string& id) const = 0;
    virtual void add(const ProductCategory& category) = 0;
};

#endif // POS_ICATEGORY_REPOSITORY_HPP
