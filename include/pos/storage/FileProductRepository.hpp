#ifndef POS_FILEPRODUCTREPOSITORY_HPP
#define POS_FILEPRODUCTREPOSITORY_HPP

#include <optional>
#include <string>
#include <vector>
#include "pos/storage/IProductRepository.hpp"

// keeps the products in a text file (Product.txt)
class FileProductRepository : public IProductRepository {
public:
    explicit FileProductRepository(const std::string& path);

    // reads the whole file
    // error if a line is wrong or an id or barcode is used twice
    std::vector<Product> all() const override;

    std::optional<Product> findByBarcode(const std::string& barcode) const override;
    std::optional<Product> findById(int id) const override;

    // changes the stock of one product and saves the file
    void updateQuantity(int id, double newQuantity) override;

    // adds a new product to the file; throws PosError if id or barcode is already used
    void add(const Product& product) override;

    // updates an existing product's fields and rewrites the file
    void update(const Product& product) override;

private:
    std::string filePath;
};

#endif