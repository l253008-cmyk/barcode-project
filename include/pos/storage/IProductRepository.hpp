#ifndef POS_IPRODUCTREPOSITORY_HPP
#define POS_IPRODUCTREPOSITORY_HPP

#include <optional>
#include <string>
#include <vector>
#include "pos/domain/Product.hpp"

// the rest of the program only uses this, so the products can be kept
// in a file now and in a database later
class IProductRepository {
public:
    virtual ~IProductRepository() {}

    virtual std::vector<Product> all() const = 0;

    // these two give an empty result when the product is not found
    virtual std::optional<Product> findByBarcode(const std::string& barcode) const = 0;
    virtual std::optional<Product> findById(int id) const = 0;

    virtual void updateQuantity(int id, double newQuantity) = 0;
};

#endif