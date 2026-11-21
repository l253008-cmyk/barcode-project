#include "pos/tools/BarcodeAssetGenerator.hpp"

#include <iostream>

#include "barcode/encoder.hpp"
#include "pos/domain/Product.hpp"
#include "pos/storage/IProductRepository.hpp"

BarcodeAssetGenerator::BarcodeAssetGenerator(IProductRepository& products,
                                             const std::string& outputFolder)
    : products_(products), outputFolder_(outputFolder) {}

int BarcodeAssetGenerator::generateAll() {
    int count = 0;
    auto all = products_.all();
    for (const Product& product : all) {
        std::string fileName = product.barcodePath;
        std::string::size_type slash = fileName.find_last_of("/\\");
        if (slash != std::string::npos) {
            fileName = fileName.substr(slash + 1);
        }
        if (fileName.empty()) {
            fileName = product.barcode + ".bmp";
        }
        std::string path = outputFolder_ + "/" + fileName;

        generateToFile(product.barcode, path);
        std::cout << "Created " << path << '\n';
        ++count;
    }
    return count;
}
