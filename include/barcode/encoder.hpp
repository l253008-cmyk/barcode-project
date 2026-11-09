// encoder.hpp
// Turns a 6-digit number into a barcode picture.

#pragma once
#include <string>
#include "barcode/image.hpp"

// Makes the barcode picture for a 6-digit code.
// Throws BarcodeError if the code is not exactly 6 digits.
Image generateBarcode(const std::string& code);
// Makes the barcode picture and saves it in the file at "path".
// Throws BarcodeError if the code is wrong, the path is empty,
// or the file cannot be saved.
void generateToFile(const std::string& code, const std::string& path);