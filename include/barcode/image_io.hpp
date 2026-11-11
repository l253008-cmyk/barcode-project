#ifndef BARCODE_IMAGE_IO_HPP
#define BARCODE_IMAGE_IO_HPP

#include <string>
#include "image.hpp"

// Read an image from a file
Image loadImage(const std::string& path);

// Save an image to a file
void saveImage(const Image& img, const std::string& path);

#endif