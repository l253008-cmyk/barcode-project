// ProductService.cpp                                  Owner: Person 5
// Implementation of ProductService for inventory, barcode generation, and product updates.

#include "pos/services/ProductService.hpp"

#include <filesystem>
#include <iomanip>
#include <sstream>

#include "barcode/encoder.hpp"
#include "barcode/validator.hpp"
#include "pos/common/PosError.hpp"
#include "pos/domain/Product.hpp"

using namespace std;

static Product findProductOrThrow(const IProductRepository& repository, const string& barcode) {
    optional<Product> found = repository.findByBarcode(barcode);
    if (!found.has_value()) {
        throw PosError("Product not found for barcode: " + barcode);
    }
    return found.value();
}

ProductService::ProductService(IProductRepository& repository, const StockPolicy& policy)
    : productRepository(repository), stockPolicy(policy) {
}

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

bool ProductService::canSell(const string& barcode, double quantity) const {
    Product product = findProductOrThrow(productRepository, barcode);
    return stockPolicy.canFulfil(product, quantity);
}

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

string ProductService::nextBarcodeSerial(const string& categoryId) const {
    if (categoryId.size() != 2 || categoryId[0] < '1' || categoryId[0] > '9' || categoryId[1] < '0' || categoryId[1] > '9') {
        throw PosError("Category ID must be 2 digits between 10 and 99: '" + categoryId + "'");
    }

    vector<Product> products = productRepository.all();
    int maxSerial = 0;
    for (const auto& p : products) {
        if (p.barcode.size() == 6 && p.barcode.substr(0, 2) == categoryId) {
            try {
                int s = stoi(p.barcode.substr(2, 4));
                if (s > maxSerial) {
                    maxSerial = s;
                }
            } catch (...) {}
        }
    }

    if (maxSerial >= 9999) {
        throw PosError("Serials in category '" + categoryId + "' are exhausted (limit 9999 reached)");
    }

    ostringstream ss;
    ss << setw(4) << setfill('0') << (maxSerial + 1);
    return ss.str();
}

string ProductService::generateBarcode(const string& categoryId) const {
    return categoryId + nextBarcodeSerial(categoryId);
}

Product ProductService::addProduct(const string& categoryId, const string& name,
                                   const string& acronym, MeasurementUnit unit,
                                   Money price, Money tax, Money discount, double startingQty) {
    if (categoryId.size() != 2 || categoryId[0] < '1' || categoryId[0] > '9' || categoryId[1] < '0' || categoryId[1] > '9') {
        throw PosError("Category ID must be 2 digits between 10 and 99: '" + categoryId + "'");
    }
    if (name.empty()) {
        throw PosError("Product name cannot be empty");
    }
    if (acronym.empty() || acronym.size() > 8) {
        throw PosError("Product acronym must be between 1 and 8 characters");
    }
    if (startingQty < 0) {
        throw PosError("Initial quantity cannot be negative");
    }

    // Check acronym uniqueness
    vector<Product> existing = allProducts();
    for (const auto& p : existing) {
        if (p.acronym == acronym) {
            throw PosError("Product acronym '" + acronym + "' already exists");
        }
    }

    string barcode = generateBarcode(categoryId);
    int id = nextProductId();
    string barcodePath = "data/barcodes/" + barcode + ".bmp";

    try {
        filesystem::create_directories("data/barcodes");
        generateToFile(barcode, barcodePath);
    } catch (...) {}

    Product toSave{id, categoryId, barcode, acronym, name, unit, price, tax, discount, startingQty, barcodePath};
    productRepository.add(toSave);
    return toSave;
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

    // Check acronym uniqueness
    vector<Product> existing = allProducts();
    for (const auto& p : existing) {
        if (p.acronym == product.acronym) {
            throw PosError("Product acronym '" + product.acronym + "' already exists");
        }
    }

    string barcodePath = product.barcodePath;
    if (barcodePath.empty()) {
        barcodePath = "data/barcodes/" + product.barcode + ".bmp";
    }

    try {
        filesystem::create_directories("data/barcodes");
        generateToFile(product.barcode, barcodePath);
    } catch (...) {}

    Product toSave = product;
    toSave.barcodePath = barcodePath;
    if (toSave.categoryId.empty() && toSave.barcode.size() >= 2) {
        toSave.categoryId = toSave.barcode.substr(0, 2);
    }
    productRepository.add(toSave);
}

void ProductService::updateProduct(const Product& product) {
    optional<Product> existingOpt = productRepository.findById(product.id);
    if (!existingOpt.has_value()) {
        throw PosError("Product with ID " + to_string(product.id) + " not found to update");
    }
    const Product& existing = existingOpt.value();

    // Fixed fields check (confirm rule: acronym, barcode, category, unit cannot be modified)
    if (product.acronym != existing.acronym) {
        throw PosError("Product acronym cannot be modified after creation");
    }
    if (product.barcode != existing.barcode) {
        throw PosError("Product barcode cannot be modified after creation");
    }
    if (!product.categoryId.empty() && !existing.categoryId.empty() && product.categoryId != existing.categoryId) {
        throw PosError("Product category cannot be modified after creation");
    }
    if (product.unit != existing.unit) {
        throw PosError("Product unit cannot be modified after creation");
    }

    if (product.name.empty()) {
        throw PosError("Product name cannot be empty");
    }
    if (product.availableQty < 0) {
        throw PosError("Available quantity cannot be negative");
    }

    productRepository.update(product);
}

void ProductService::updateStock(int id, double newQuantity) {
    if (newQuantity < 0) {
        throw PosError("Available stock cannot be negative");
    }
    optional<Product> found = productRepository.findById(id);
    if (!found.has_value()) {
        throw PosError("Product ID " + to_string(id) + " not found");
    }
    productRepository.updateQuantity(id, newQuantity);
}
