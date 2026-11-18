// IBarcodeReader.hpp                                 Owner: Person 1
// Reads a barcode from one frame. The scan controller only knows this interface,
// so another barcode type can be added later without changing it.

#ifndef POS_IBARCODE_READER_HPP
#define POS_IBARCODE_READER_HPP

#include <optional>
#include <string>

#include "barcode/image.hpp"

class IBarcodeReader {
public:
    virtual ~IBarcodeReader() = default;

    // Returns the code found in the frame, or nothing if there is no readable barcode.
    // Never throws for an unreadable frame, because most camera frames have no barcode.
    virtual std::optional<std::string> read(const Image& frame) = 0;
};

#endif  // POS_IBARCODE_READER_HPP