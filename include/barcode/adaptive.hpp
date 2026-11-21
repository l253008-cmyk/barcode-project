#ifndef BARCODE_ADAPTIVE_HPP
#define BARCODE_ADAPTIVE_HPP

#include "barcode/image.hpp"

// Converts a grayscale image to pure black (0) and white (255).
// Every pixel is compared with the average of the pixels around it in the
// same row, so shadows and uneven light do not break the barcode.
// Use it when the global average of toBinary is not good enough (camera frames).
// Throws BarcodeError for an empty or damaged image.
Image toBinaryAdaptive(const Image& gray);

#endif // BARCODE_ADAPTIVE_HPP