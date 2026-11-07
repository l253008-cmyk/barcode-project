#ifndef BARCODE_PREPROCESS_HPP
#define BARCODE_PREPROCESS_HPP

#include "barcode/image.hpp"

// Converts a grayscale image to pure black (0) and white (255)
// using a threshold chosen from the image itself.
Image toBinary(const Image& gray);

#endif // BARCODE_PREPROCESS_HPP