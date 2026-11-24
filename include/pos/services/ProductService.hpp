// ProductService.hpp                                  Owner: Person 5
// High-level service for product inventory, barcode generation, and updates.

#ifndef POS_PRODUCTSERVICE_HPP
#define POS_PRODUCTSERVICE_HPP

#include <string>
#include <vector>

#include "pos/domain/Product.hpp"
#include "pos/services/StockPolicy.hpp"
#include "pos/storage/IProductRepository.hpp"

class ProductService {
public:
    ProductService(IProductRepository& repository, const StockPolicy& policy);

    // All details of the product as text for the screen
    std::string describeByBarcode(const std::string& barcode) const;

    // True if the stock allows this quantity
    bool canSell(const std::string& barcode, double quantity) const;

    // Takes the sold quantity out of the stock
    void reduceStock(const std::string& barcode, double quantity);

    // Returns all products in the catalog
    std::vector<Product> allProducts() const;
    std::vector<Product> inventory() const { return allProducts(); }

    // Suggests the next available product ID
    int nextProductId() const;

    // Generates the next 4-digit serial for the given category (0001 up to 9999)
    std::string nextBarcodeSerial(const std::string& categoryId) const;

    // Generates 6-digit barcode for a category: categoryId (2 digits) + 4-digit serial
    std::string generateBarcode(const std::string& categoryId) const;

    // Adds a new product: builds barcode, validates acronym uniqueness, generates BMP, saves
    Product addProduct(const std::string& categoryId, const std::string& name,
                       const std::string& acronym, MeasurementUnit unit,
                       Money price, Money tax, Money discount, double startingQty);

    // Overload for direct Product struct
    void addProduct(const Product& product);

    // Updates an existing product (acronym, barcode, category, unit remain fixed)
    void updateProduct(const Product& product);

    // Directly updates the available stock quantity for a product
    void updateStock(int id, double newQuantity);

private:
    IProductRepository& productRepository;
    const StockPolicy& stockPolicy;
};

#endif // POS_PRODUCTSERVICE_HPP