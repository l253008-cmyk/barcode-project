#ifndef BARCODE_ERROR_HPP
#define BARCODE_ERROR_HPP

#include <stdexcept>
#include <string>

// Our own error type for barcode-related problems
class BarcodeError : public std::runtime_error
{
public:
    BarcodeError(const std::string& message)
        : std::runtime_error(message)
    {
    }
};

#endif