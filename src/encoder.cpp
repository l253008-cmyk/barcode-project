// encoder.cpp
// Joins the steps of Part 1: code -> 13 digits -> 95 characters -> picture -> file.

#include "barcode/encoder.hpp"
#include "barcode/config.hpp"
#include "barcode/ean13.hpp"
#include "barcode/error.hpp"
#include "barcode/image_io.hpp"
#include "barcode/renderer.hpp"

using namespace std;

// A wrong code makes toEan13 throw, so nothing is drawn.
Image generateBarcode(const string& code) {
    string ean13 = toEan13(code);
    string modules = buildModules(ean13);
    Image barcodeImage = renderModules(modules, kModuleWidthPx, kBarHeightPx, kQuietZoneModules);
    return barcodeImage;
}

void generateToFile(const string& code, const string& path) {
    if (path.empty()) {
        throw BarcodeError("generateToFile: path is empty");
    }
    Image barcodeImage = generateBarcode(code);
    saveImage(path, barcodeImage);
}