// test_encoder.cpp                                   Owner: Person 2
// Tests for encoder.cpp (number -> barcode picture -> file).
// Needs Person 3's validator.cpp, renderer.cpp and image_io.cpp to run.

#include <cassert>
#include <iostream>
#include "barcode/config.hpp"
#include "barcode/encoder.hpp"
#include "barcode/error.hpp"
#include "barcode/image_io.hpp"
using namespace std;

// True when generateBarcode throws a BarcodeError.
static bool generateBarcodeThrows(const string& code) {
    try {
        generateBarcode(code);
    }
    catch (const BarcodeError&) {
        return true;
    }
    return false;
}

// True when generateToFile throws a BarcodeError.
static bool generateToFileThrows(const string& code, const string& path) {
    try {
        generateToFile(code, path);
    }
    catch (const BarcodeError&) {
        return true;
    }
    return false;
}

int main() {
    // ---------- size of the picture ----------
    Image barcodeImage = generateBarcode("123456");
    int expectedWidth = (95 + 2 * kQuietZoneModules) * kModuleWidthPx;
    assert(barcodeImage.width == expectedWidth);
    assert(barcodeImage.height == kBarHeightPx);

    // ---------- colours in the picture ----------
    // Quiet zone is white. Then the start guard "101":
    // module 0 is black, module 1 is white.
    int middleRow = barcodeImage.height / 2;
    int firstBarX = kQuietZoneModules * kModuleWidthPx;
    int secondModuleX = firstBarX + kModuleWidthPx;
    assert(barcodeImage.at(0, middleRow) == 255);
    assert(barcodeImage.at(firstBarX, middleRow) == 0);
    assert(barcodeImage.at(secondModuleX, middleRow) == 255);

    // ---------- other valid codes ----------
    Image smallestImage = generateBarcode("100000");
    assert(smallestImage.width == expectedWidth);
    Image ninesImage = generateBarcode("999999");
    assert(ninesImage.width == expectedWidth);

    // ---------- wrong codes must throw ----------
    assert(generateBarcodeThrows(""));
    assert(generateBarcodeThrows("12345"));        // too short
    assert(generateBarcodeThrows("1234567"));      // too long
    assert(generateBarcodeThrows("12a456"));       // a letter
    assert(generateBarcodeThrows("000000"));       // starts with 0
    assert(generateBarcodeThrows("012345"));       // starts with 0

    // ---------- generateToFile ----------
    assert(generateToFileThrows("123456", ""));                  // empty path
    assert(generateToFileThrows("12345", "test_output.pgm"));    // wrong code

    // Save a barcode, read it back, and check it is the same picture.
    string testPath = "test_output.pgm";
    generateToFile("123456", testPath);
    Image savedImage = loadImage(testPath);
    assert(savedImage.width == barcodeImage.width);
    assert(savedImage.height == barcodeImage.height);
    assert(savedImage.pixels == barcodeImage.pixels);

    cout << "test_encoder: all passed" << endl;
    return 0;
}