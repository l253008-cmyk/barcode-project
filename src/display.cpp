#include "barcode/display.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

#include "barcode/config.hpp"
#include "barcode/error.hpp"
#include "barcode/image.hpp"
#include "barcode/image_io.hpp"

using namespace std;

// Prints the barcode in the console like a real barcode:
// a white label (solid blocks) with black bars (spaces) on it.
// Only this file uses it, so it is kept private.
static void printTextPreview(const Image& img) {
    const int step = kModuleWidthPx;   // pixels in one module
    const int row = img.height / 2;    // all rows are the same, so use the middle one
    const int barLines = 6;            // how many text lines tall the bars are

#ifdef _WIN32
    const char white = static_cast<char>(219);   // full block in the Windows console
#else
    const char white = '#';
#endif

    string bars = "";
    for (int x = step / 2; x < img.width; x = x + step) {
        if (img.at(x, row) < 128) {
            bars = bars + " ";         // black bar
        }
        else {
            bars = bars + white;       // white background
        }
    }
    string label(bars.size(), white);  // plain white line for the top and bottom edge

    cout << "\nBarcode preview:\n" << label << "\n";
    for (int i = 0; i < barLines; i++) {
        cout << bars << "\n";
    }
    cout << label << "\n";
}

void showImage(const Image& img) {
    if (img.width <= 0 || img.height <= 0) {
        throw BarcodeError("Cannot display an empty image.");
    }

    printTextPreview(img);

    string fileName = "barcode_preview.bmp";
    saveImage(img, fileName);

#ifdef _WIN32
    string command = "start \"\" \"" + fileName + "\"";
#elif defined(__APPLE__)
    string command = "open \"" + fileName + "\"";
#else
    string command = "xdg-open \"" + fileName + "\" > /dev/null 2>&1";
#endif

    int result = system(command.c_str());
    if (result != 0) {
        throw BarcodeError("Could not open the image viewer.");
    }
}