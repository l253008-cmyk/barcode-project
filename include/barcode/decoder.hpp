#ifndef BARCODE_DECODER_HPP
#define BARCODE_DECODER_HPP

#include "barcode/image.hpp"
#include <string>

// Decodes a barcode from an image.
// Throws BarcodeError if no valid barcode can be read.
std::string decodeBarcode(const Image& img);

// Loads an image from a file and decodes its barcode.
std::string decodeFromFile(const std::string& path);

#endif // BARCODE_DECODER_HPP