// Ean13Reader.cpp                                     Owner: Person 1
// Implements barcode reading from camera frames.

#include "pos/scanning/Ean13Reader.hpp"

#include "barcode/camera_decoder.hpp"
#include "barcode/error.hpp"
#include "barcode/validator.hpp"

std::optional<std::string> Ean13Reader::read(const Image& frame) {
    try {
        std::string code = decodeFrame(frame);
        if (isValidCode(code)) {
            return code;
        }
    } catch (const BarcodeError&) {
        // Frame does not contain a recognizable barcode; return nothing
    } catch (...) {
        // Catch any unexpected exceptions and safely return nothing
    }
    return std::nullopt;
}
