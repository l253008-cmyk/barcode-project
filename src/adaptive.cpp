﻿// adaptive.cpp                                        Owner: Person 4
// Local adaptive thresholding for camera frames with uneven lighting and shadows.

#include "barcode/adaptive.hpp"
#include "barcode/error.hpp"

Image toBinaryAdaptive(const Image& gray) {
    if (gray.width <= 0 || gray.height <= 0 ||
        gray.pixels.size() != static_cast<size_t>(gray.width) * gray.height) {
        throw BarcodeError("Invalid or empty image");
    }

    int radius = gray.width / 8;
    if (radius < 7) {
        radius = 7;
    }

    const int offset = 7;

    Image binary;
    binary.width = gray.width;
    binary.height = gray.height;
    binary.pixels.resize(gray.pixels.size());

    for (int y = 0; y < gray.height; ++y) {
        long long windowSum = 0;
        int left = 0;
        int right = -1;

        for (int x = 0; x < gray.width; ++x) {
            int newLeft = x - radius;
            if (newLeft < 0) newLeft = 0;
            int newRight = x + radius;
            if (newRight > gray.width - 1) newRight = gray.width - 1;

            while (right < newRight) {
                ++right;
                windowSum += gray.at(right, y);
            }
            while (left < newLeft) {
                windowSum -= gray.at(left, y);
                ++left;
            }

            long long average = windowSum / (right - left + 1);

            if (gray.at(x, y) < average - offset) {
                binary.at(x, y) = 0;
            }
            else {
                binary.at(x, y) = 255;
            }
        }
    }
    return binary;
}