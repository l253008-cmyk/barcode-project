#include "pos/services/ProductService.hpp"
#include "pos/common/PosError.hpp"
#include "pos/domain/Product.hpp"

using namespace std;

// look for the product, throw an error if it is not there
static Product findProductOrThrow(const IProductRepository& repository,
    const string& barcode) {
    optional<Product> found = repository.findByBarcode(barcode);
    if (!found.has_value()) {
        throw PosError("Product not found for barcode: " + barcode);
    }
    return found.value();
}

ProductService::ProductService(IProductRepository& repository, const StockPolicy& policy)
    : productRepository(repository), stockPolicy(policy) {
}

// one line for every field of the product
string ProductService::describeByBarcode(const string& barcode) const {
    Product product = findProductOrThrow(productRepository, barcode);

    string text;
    text += "Name: " + product.name + "\n";
    text += "Acronym: " + product.acronym + "\n";
    text += "Barcode: " + product.barcode + "\n";
    text += "Unit: " + unitToText(product.unit) + "\n";
    text += "Unit price: " + product.unitPrice.toString() + "\n";
    text += "Tax: " + product.tax.toString() + "\n";
    text += "Discount: " + product.discount.toString() + "\n";
    text += "Available: " + quantityToText(product.availableQty);
    return text;
}

// the policy decides if the stock is enough
bool ProductService::canSell(const string& barcode, double quantity) const {
    Product product = findProductOrThrow(productRepository, barcode);
    return stockPolicy.canFulfil(product, quantity);
}

// called after a sale, the sold quantity goes out of stock
void ProductService::reduceStock(const string& barcode, double quantity) {
    Product product = findProductOrThrow(productRepository, barcode);

    if (!stockPolicy.canFulfil(product, quantity)) {
        throw PosError("Cannot sell " + quantityToText(quantity) + " of " + product.acronym +
            " (available: " + quantityToText(product.availableQty) + ")");
    }

    double newQuantity = product.availableQty - quantity;
    productRepository.updateQuantity(product.id, newQuantity);
}