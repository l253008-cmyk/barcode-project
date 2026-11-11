#include "barcode/decoder.hpp"
#include "barcode/encoder.hpp"
#include "test_utils.hpp"
#include <string>
using namespace std;

// Decodes an image; returns "(exception)" if decoding fails.
static string tryDecode(const Image& img) {
    try {
        return decodeBarcode(img);
    }
    catch (const BarcodeError&) {
        return "(exception)";
    }
}

int main() {
    bool ok = true;

    // 1. An empty image must throw.
    Image empty;
    ok &= checkThrows("Empty image test", [&]() { decodeBarcode(empty); });

    // 2. A blank white image (no barcode) must throw.
    Image blank;
    blank.width = 100;
    blank.height = 50;
    blank.pixels.resize(5000, 255);
    ok &= checkThrows("No barcode test", [&]() { decodeBarcode(blank); });

    // 3. A barcode made by the encoder must decode to the same code.
    const string code = "123456";
    Image barcode = generateBarcode(code);
    ok &= checkResult("Valid barcode test", code, tryDecode(barcode));

    // 4. The same barcode flipped left to right (upside down) must also decode.
    Image flipped = barcode;
    for (int y = 0; y < barcode.height; y++) {
        for (int x = 0; x < barcode.width; x++) {
            flipped.at(x, y) = barcode.at(barcode.width - 1 - x, y);
        }
    }
    ok &= checkResult("Upside-down barcode test", code, tryDecode(flipped));

    return ok ? 0 : 1;
}
