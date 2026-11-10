#include "barcode/display.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

#include "barcode/config.hpp"
#include "barcode/error.hpp"
#include "barcode/image.hpp"
#include "barcode/image_io.hpp"

using namespace std;

void showImage(const Image& img) {
    if (img.width <= 0 || img.height <= 0) {
        throw BarcodeError("Cannot display an empty image.");
    }

   
    int row = img.height / 2;
    int step = kModuleWidthPx;

    string line = "";
    for (int x = step / 2; x < img.width; x = x + step) {
        if (img.at(x, row) < 128) {
            line = line + "#";
        } else {
            line = line + " ";
        }
    }

    cout << "\nText preview:\n";
    for (int i = 0; i < 3; i++) {
        cout << line << "\n";
    }

    
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
