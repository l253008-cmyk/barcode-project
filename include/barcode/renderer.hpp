#ifndef BARCODE_RENDERER_HPP
#define BARCODE_RENDERER_HPP

#include <string>
#include "image.hpp"

// Convert barcode modules into an image
Image renderModules(
    const std::string& modules,
    int moduleWidth,
    int height,
    int quietZone
);

#endif