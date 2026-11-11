#include "barcode/image_io.hpp"
#include "barcode/image.hpp"

#include <iostream>
#include <string>

int main()
{
    // Create a small test image.
    Image original;

    original.width = 3;
    original.height = 2;

    // Six pixels for a 3x2 image.
    original.pixels.push_back(0);
    original.pixels.push_back(255);
    original.pixels.push_back(100);

    original.pixels.push_back(255);
    original.pixels.push_back(0);
    original.pixels.push_back(200);

    // Test saving and loading a PGM image.
    saveImage(original, "test_image.pgm");

    Image pgmImage = loadImage("test_image.pgm");

    if (pgmImage.width != original.width ||
        pgmImage.height != original.height)
    {
        std::cout << "PGM dimensions test failed\n";
        return 1;
    }

    // Compare every pixel.
    for (std::size_t i = 0; i < original.pixels.size(); i++)
    {
        if (pgmImage.pixels[i] != original.pixels[i])
        {
            std::cout << "PGM pixel test failed\n";
            return 1;
        }
    }

    // Test saving and loading a BMP image.
    saveImage(original, "test_image.bmp");

    Image bmpImage = loadImage("test_image.bmp");

    if (bmpImage.width != original.width ||
        bmpImage.height != original.height)
    {
        std::cout << "BMP dimensions test failed\n";
        return 1;
    }

    // Compare every pixel.
    for (std::size_t i = 0; i < original.pixels.size(); i++)
    {
        if (bmpImage.pixels[i] != original.pixels[i])
        {
            std::cout << "BMP pixel test failed\n";
            return 1;
        }
    }

    std::cout << "All image I/O tests passed\n";

    return 0;
}