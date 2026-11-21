#include <iostream>
#include <string>

#include "barcode/error.hpp"
#include "pos/common/PosError.hpp"
#include "pos/storage/FileProductRepository.hpp"
#include "pos/tools/BarcodeAssetGenerator.hpp"

int main(int argc, char* argv[]) {
#ifdef POS_DATA_DIR
    std::string dataDir = POS_DATA_DIR;
#else
    std::string dataDir = "data";
#endif
    if (argc > 1) {
        dataDir = argv[1];
    }

    try {
        FileProductRepository products(dataDir + "/Product.txt");
        BarcodeAssetGenerator generator(products, dataDir + "/barcodes");
        int count = generator.generateAll();
        std::cout << count << " barcode images written to " << dataDir << "/barcodes\n";
        return 0;
    } catch (const PosError& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    } catch (const BarcodeError& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
