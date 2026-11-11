#include "barcode/validator.hpp"
#include "barcode/error.hpp"

#include <iostream>
#include <string>

int main()
{
    // These codes should be accepted.
    std::string validCodes[] = {
        "123456",
        "999999",
        "100000"
    };

    // These codes should be rejected.
    std::string invalidCodes[] = {
        "12345",
        "1234567",
        "12a456",
        "12 456",
        "",
        "012345",
        "000000"
    };

    // Test valid codes.
    for (std::size_t i = 0; i < 3; i++)
    {
        if (!isValidCode(validCodes[i]))
        {
            std::cout << "Valid code test failed\n";
            return 1;
        }
    }

    // Test invalid codes.
    for (std::size_t i = 0; i < 7; i++)
    {
        if (isValidCode(invalidCodes[i]))
        {
            std::cout << "Invalid code test failed\n";
            return 1;
        }
    }

    // Check that requireValidCode throws an error.
    try
    {
        requireValidCode("12345a");

        std::cout << "Error test failed\n";
        return 1;
    }
    catch (const BarcodeError&)
    {
        // Expected error.
    }

    // Check that a valid code does not throw an error.
    try
    {
        requireValidCode("123456");
    }
    catch (const BarcodeError&)
    {
        std::cout << "Valid code threw an error\n";
        return 1;
    }

    std::cout << "All validator tests passed\n";

    return 0;
}