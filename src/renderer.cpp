#include "barcode/renderer.hpp"
#include "barcode/config.hpp"
#include "barcode/error.hpp"
#include "barcode/image.hpp"

#include <string>

// Converts the module string into a barcode image.
Image renderModules(const std::string& modules,
    int moduleWidth,
    int height,
    int quietZone)
{
    if (modules.empty())
    {
        throw BarcodeError("Barcode modules cannot be empty");
    }

    if (moduleWidth <= 0 || height <= 0 || quietZone < 0)
    {
        throw BarcodeError("Invalid barcode dimensions");
    }

    // Check that modules contain only 0 and 1.
    for (std::size_t i = 0; i < modules.length(); i++)
    {
        if (modules[i] != '0' && modules[i] != '1')
        {
            throw BarcodeError("Modules can contain only 0 and 1");
        }
    }

    // Calculate total image width, including quiet zones.
    int totalModules = static_cast<int>(modules.length()) + 2 * quietZone;
    int imageWidth = totalModules * moduleWidth;

    Image img;
    img.width = imageWidth;
    img.height = height;

    // Start with a completely white image.
    img.pixels.assign(
        static_cast<std::size_t>(imageWidth) * height, 255
    );

    // Draw every black module.
    for (std::size_t i = 0; i < modules.length(); i++)
    {
        if (modules[i] == '1')
        {
            // Find the starting x-position of this bar.
            int startX = (static_cast<int>(i) + quietZone) * moduleWidth;

            // Draw the bar from top to bottom.
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < moduleWidth; x++)
                {
                    img.at(startX + x, y) = 0;
                }
            }
        }
    }

    return img;
}