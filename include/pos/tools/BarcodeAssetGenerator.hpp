#pragma once

#include <string>

class IProductRepository;

class BarcodeAssetGenerator {
public:
    BarcodeAssetGenerator(IProductRepository& products, const std::string& outputFolder);

    int generateAll();

private:
    IProductRepository& products_;
    std::string outputFolder_;
};
