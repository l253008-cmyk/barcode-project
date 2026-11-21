#ifndef POS_PRODUCTSERVICE_HPP
#define POS_PRODUCTSERVICE_HPP

#include <string>
#include "pos/services/StockPolicy.hpp"
#include "pos/storage/IProductRepository.hpp"

// the things we do with products: show details, check stock, reduce stock
class ProductService {
public:
    ProductService(IProductRepository& repository, const StockPolicy& policy);

    // all details of the product as text for the screen
    // error if the barcode is not found
    std::string describeByBarcode(const std::string& barcode) const;

    // true if the stock allows this quantity, error if the barcode is not found
    bool canSell(const std::string& barcode, double quantity) const;

    // takes the sold quantity out of the stock, error if it is not allowed
    void reduceStock(const std::string& barcode, double quantity);

private:
    IProductRepository& productRepository;
    const StockPolicy& stockPolicy;
};

#endif