#include "barcode/preprocess.hpp"
#include "barcode/error.hpp"

Image toBinary(const Image& gray) {
    if (gray.width <= 0 || gray.height <= 0 ||
        gray.pixels.size() != static_cast<size_t>(gray.width) * gray.height) {
        throw BarcodeError("Invalid or empty image");
    }

    // Threshold = average pixel value of the whole image.
    long long sum = 0;
    for (int y = 0; y < gray.height; y++) {
        for (int x = 0; x < gray.width; x++) {
            sum += gray.at(x, y);
        }
    }
    int threshold = static_cast<int>(sum / static_cast<long long>(gray.pixels.size()));

    Image binary;
    binary.width = gray.width;
    binary.height = gray.height;
    binary.pixels.resize(gray.pixels.size());

    for (int y = 0; y < gray.height; y++) {
        for (int x = 0; x < gray.width; x++) {
            if (gray.at(x, y) < threshold) {
                binary.at(x, y) = 0;
            }
            else {
                binary.at(x, y) = 255;
            }
        }
    }
    return binary;
}