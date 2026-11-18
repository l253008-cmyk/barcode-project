// Ean13Reader.hpp                                     Owner: Person 1
// Uses decodeFrame to read EAN-13 barcodes from camera frames.
// Implements IBarcodeReader.

#ifndef POS_EAN13_READER_HPP
#define POS_EAN13_READER_HPP

#include <optional>
#include <string>

#include "barcode/image.hpp"
#include "pos/scanning/IBarcodeReader.hpp"

class Ean13Reader : public IBarcodeReader {
public:
    std::optional<std::string> read(const Image& frame) override;
};

#endif  // POS_EAN13_READER_HPP
