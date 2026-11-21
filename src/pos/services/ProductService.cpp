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

vector<Product> ProductService::allProducts() const {
    return productRepository.all();
}

int ProductService::nextProductId() const {
    vector<Product> products = productRepository.all();
    int maxId = 0;
    for (const auto& p : products) {
        if (p.id > maxId) {
            maxId = p.id;
        }
    }
    return maxId + 1;
}

void ProductService::addProduct(const Product& product) {
    if (product.id <= 0) {
        throw PosError("Product ID must be greater than zero");
    }
    if (product.barcode.empty()) {
        throw PosError("Barcode cannot be empty");
    }
    if (product.acronym.empty() || product.acronym.size() > 8) {
        throw PosError("Product acronym must be between 1 and 8 characters");
    }
    if (product.name.empty()) {
        throw PosError("Product name cannot be empty");
    }
    if (product.availableQty < 0) {
        throw PosError("Available quantity cannot be negative");
    }
    if (productRepository.findByBarcode(product.barcode).has_value()) {
        throw PosError("Product with barcode " + product.barcode + " already exists");
    }
    if (productRepository.findById(product.id).has_value()) {
        throw PosError("Product with ID " + to_string(product.id) + " already exists");
    }

    // Auto-generate the barcode image file
    string barcodePath = product.barcodePath;
    if (barcodePath.empty()) {
        barcodePath = "data/barcodes/" + product.barcode + ".bmp";
    }

    Product toSave = product;
    toSave.barcodePath = barcodePath;
    productRepository.add(toSave);
}