#ifndef BARCODE_IMAGE_HPP
#define BARCODE_IMAGE_HPP

#include <vector>

// Stores a grayscale image
struct Image
{
    int width = 0;
    int height = 0;

    // Each pixel has a value from 0 to 255
    // 0 means black and 255 means white
    std::vector<unsigned char> pixels;

    // Get or change a pixel
    unsigned char& at(int x, int y)
    {
        return pixels[y * width + x];
    }

    // Read a pixel without changing it
    const unsigned char& at(int x, int y) const
    {
        return pixels[y * width + x];
    }
};

#endif