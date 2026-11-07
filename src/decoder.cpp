#include "barcode/decoder.hpp"
#include "barcode/preprocess.hpp"
#include "barcode/scanner.hpp"
#include "barcode/ean13.hpp"
#include "barcode/image_io.hpp"
#include "barcode/error.hpp"
#include <vector>
#include <string>
using namespace std;

// Tries to turn one row of run lengths into a 6-digit code.
// Returns "" on any failure so the caller can try another row.
static string checkRuns(const vector<int>& runs) {
    try {
        int startIndex = 0;
        double moduleWidth = 0;

        if (!findStartGuard(runs, startIndex, moduleWidth)) {
            return "";
        }

        string ean13 = decodeRuns(runs, startIndex, moduleWidth);
        if (ean13.empty()) {
            return "";
        }

        return fromEan13(ean13);
    }
    catch (const BarcodeError&) {
        return "";
    }
}

// Tries one pixel row normally, then the same row reversed
// (for upside-down barcodes).
static string tryRow(const Image& img, int row) {
    vector<int> runs = scanRow(img, row);
    string result = checkRuns(runs);

    if (!result.empty()) {
        return result;
    }

    Image reversed;
    reversed.width = img.width;
    reversed.height = 1;
    reversed.pixels.resize(img.width);

    for (int x = 0; x < img.width; x++) {
        reversed.at(x, 0) = img.at(img.width - 1 - x, row);
    }

    runs = scanRow(reversed, 0);
    return checkRuns(runs);
}

// Keeps a row number inside the image (small images can push it outside).
static int clampRow(int row, int height) {
    if (row < 0) return 0;
    if (row >= height) return height - 1;
    return row;
}

string decodeBarcode(const Image& img) {
    Image binary = toBinary(img);

    int middle = binary.height / 2;
    int offset10 = binary.height / 10;
    int offset20 = binary.height / 5;

    // Offsets must be at least 1 row for very small images.
    if (offset10 < 1) offset10 = 1;
    if (offset20 < 1) offset20 = 1;

    // Middle row first, then 10% and 20% above and below.
    const int rows[] = { middle,
                         middle - offset10, middle + offset10,
                         middle - offset20, middle + offset20 };

    for (int row : rows) {
        string result = tryRow(binary, clampRow(row, binary.height));
        if (!result.empty()) {
            return result;
        }
    }

    throw BarcodeError("Could not decode barcode");
}

string decodeFromFile(const string& path) {
    Image img = loadImage(path);
    return decodeBarcode(img);
}