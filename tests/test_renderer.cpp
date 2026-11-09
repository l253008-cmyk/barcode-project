#include "barcode/renderer.hpp"
#include "barcode/error.hpp"
#include "barcode/image.hpp"

#include <iostream>
#include <string>

int main()
{
    std::string modules = "1010";

    int moduleWidth = 2;
    int height = 5;
    int quietZone = 2;

    // Create the barcode image.
    Image img = renderModules(
        modules, moduleWidth, height, quietZone
    );

    // Check the image dimensions.
    int expectedWidth =
        (static_cast<int>(modules.length()) + 2 * quietZone)
        * moduleWidth;

    if (img.width != expectedWidth)
    {
        std::cout << "Width test failed\n";
        return 1;
    }

    if (img.height != height)
    {
        std::cout << "Height test failed\n";
        return 1;
    }

    // Left quiet zone should be white.
    if (img.at(0, 0) != 255)
    {
        std::cout << "Quiet zone test failed\n";
        return 1;
    }

    // First module is 1, so it should be black.
    if (img.at(quietZone * moduleWidth, 0) != 0)
    {
        std::cout << "Black bar test failed\n";
        return 1;
    }

    // Second module is 0, so it should be white.
    if (img.at((quietZone + 1) * moduleWidth, 0) != 255)
    {
        std::cout << "White bar test failed\n";
        return 1;
    }

    // Every pixel should be either black or white.
    for (std::size_t i = 0; i < img.pixels.size(); i++)
    {
        if (img.pixels[i] != 0 && img.pixels[i] != 255)
        {
            std::cout << "Pixel colour test failed\n";
            return 1;
        }
    }

    // Invalid modules should throw an error.
    try
    {
        renderModules("10A1", 2, 5, 2);

        std::cout << "Invalid module test failed\n";
        return 1;
    }
    catch (const BarcodeError&)
    {
        // Expected error.
    }

    // Invalid dimensions should throw an error.
    try
    {
        renderModules("1010", 0, 5, 2);

        std::cout << "Invalid dimension test failed\n";
        return 1;
    }
    catch (const BarcodeError&)
    {
        // Expected error.
    }

    std::cout << "All renderer tests passed\n";

    return 0;
}