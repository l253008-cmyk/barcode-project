#include "barcode/validator.hpp"
#include "barcode/config.hpp"
#include "barcode/error.hpp"

#include <string>

// Checks whether a character is a digit.
static bool isDigitChar(char ch)
{
    return ch >= '0' && ch <= '9';
}

// Checks whether the code is valid.
bool isValidCode(const std::string& code)
{
    // Code must have exactly 6 digits.
    if (code.length() != kCodeLength)
    {
        return false;
    }

    // First digit cannot be zero.
    if (code[0] == '0')
    {
        return false;
    }

    // Check every character manually.
    for (std::size_t i = 0; i < code.length(); i++)
    {
        if (!isDigitChar(code[i]))
        {
            return false;
        }
    }

    return true;
}

// Throws an error if the code is invalid.
void requireValidCode(const std::string& code)
{
    if (code.length() != kCodeLength)
    {
        throw BarcodeError("Code must contain exactly 6 digits");
    }

    if (code[0] == '0')
    {
        throw BarcodeError("First digit cannot be zero");
    }

    for (std::size_t i = 0; i < code.length(); i++)
    {
        if (!isDigitChar(code[i]))
        {
            throw BarcodeError("Code can contain digits only");
        }
    }
}