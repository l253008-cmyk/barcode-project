
#include "pos/storage/FileProductRepository.hpp"
#include "pos/common/PosError.hpp"
#include "pos/common/TextRecordFile.hpp"
#include "pos/storage/ProductCodec.hpp"
#include <filesystem>
#include <system_error>

using namespace std;

// only these two functions use TextRecordFile
static vector<vector<string>> readRecords(const string& path) {
    TextRecordFile file(path);
    return file.readAll();
}

static void writeRecords(const string& path, const vector<vector<string>>& records) {
    TextRecordFile file(path);
    file.rewrite(records);
}

FileProductRepository::FileProductRepository(const string& path)
    : filePath(path) {
}

vector<Product> FileProductRepository::all() const {
    vector<vector<string>> records = readRecords(filePath);

    vector<Product> products;
    int count = static_cast<int>(records.size());
    for (int i = 0; i < count; i++) {
        try {
            products.insert(products.end(), 1, ProductCodec::parse(records[i]));
        }
        catch (const PosError& error) {
            // tell which record is wrong so it is easy to fix
            throw PosError("Product record " + to_string(i + 1) + ": " + error.what());
        }
    }

    // two products cannot have the same id or the same barcode
    int productCount = static_cast<int>(products.size());
    for (int i = 0; i < productCount; i++) {
        for (int j = i + 1; j < productCount; j++) {
            if (products[i].id == products[j].id) {
                throw PosError("Product id " + to_string(products[i].id) + " is used twice");
            }
            if (products[i].barcode == products[j].barcode) {
                throw PosError("Product barcode " + products[i].barcode + " is used twice");
            }
        }
    }
    return products;
}

optional<Product> FileProductRepository::findByBarcode(const string& barcode) const {
    vector<Product> products = all();
    int count = static_cast<int>(products.size());
    for (int i = 0; i < count; i++) {
        if (products[i].barcode == barcode) {
            return products[i];
        }
    }
    return nullopt;
}

optional<Product> FileProductRepository::findById(int id) const {
    vector<Product> products = all();
    int count = static_cast<int>(products.size());
    for (int i = 0; i < count; i++) {
        if (products[i].id == id) {
            return products[i];
        }
    }
    return nullopt;
}

void FileProductRepository::updateQuantity(int id, double newQuantity) {
    // stock can be zero but not below zero (this also catches not-a-number)
    if (!(newQuantity >= 0)) {
        throw PosError("updateQuantity: quantity cannot be negative");
    }

    vector<Product> products = all();

    bool found = false;
    int count = static_cast<int>(products.size());
    for (int i = 0; i < count; i++) {
        if (products[i].id == id) {
            products[i].availableQty = newQuantity;
            found = true;
        }
    }
    if (!found) {
        throw PosError("updateQuantity: no product with id " + to_string(id));
    }

    vector<vector<string>> records;
    for (int i = 0; i < count; i++) {
        records.insert(records.end(), 1, ProductCodec::format(products[i]));
    }

    // save safely: write a temporary file first and then replace the real one,
    // so the real file is never left half written
    string tempPath = filePath + ".tmp";
    writeRecords(tempPath, records);

    error_code errorCode;
    filesystem::rename(tempPath, filePath, errorCode);
    if (errorCode) {
        error_code ignored;
        filesystem::remove(tempPath, ignored);
        throw PosError("Cannot save " + filePath + ": " + errorCode.message());
    }
}

void FileProductRepository::add(const Product& product) {
    vector<Product> products = all();

    for (const auto& p : products) {
        if (p.id == product.id) {
            throw PosError("Product id " + to_string(product.id) + " is already used");
        }
        if (p.barcode == product.barcode) {
            throw PosError("Product barcode " + product.barcode + " is already used");
        }
    }

    products.push_back(product);

    vector<vector<string>> records;
    for (const auto& p : products) {
        records.push_back(ProductCodec::format(p));
    }

    string tempPath = filePath + ".tmp";
    writeRecords(tempPath, records);

    error_code errorCode;
    filesystem::rename(tempPath, filePath, errorCode);
    if (errorCode) {
        error_code ignored;
        filesystem::remove(tempPath, ignored);
        throw PosError("Cannot save " + filePath + ": " + errorCode.message());
    }
}

void FileProductRepository::update(const Product& product) {
    vector<Product> products = all();
    bool found = false;
    for (auto& p : products) {
        if (p.id == product.id) {
            p = product;
            found = true;
            break;
        }
    }

    if (!found) {
        throw PosError("Product with ID " + to_string(product.id) + " not found to update");
    }

    vector<vector<string>> records;
    for (const auto& p : products) {
        records.push_back(ProductCodec::format(p));
    }

    string tempPath = filePath + ".tmp";
    writeRecords(tempPath, records);

    error_code errorCode;
    filesystem::rename(tempPath, filePath, errorCode);
    if (errorCode) {
        error_code ignored;
        filesystem::remove(tempPath, ignored);
        throw PosError("Cannot save " + filePath + ": " + errorCode.message());
    }
}


