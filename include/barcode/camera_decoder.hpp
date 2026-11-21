#ifndef BARCODE_CAMERA_DECODER_HPP
#define BARCODE_CAMERA_DECODER_HPP

#include "barcode/image.hpp"
#include <string>

// Reads a six-digit code from one camera frame (grayscale Image).
//
// A camera frame is harder than a generated barcode: the barcode is only a
// part of the frame, and the light, focus and noise are not perfect. So this
// function is more patient than decodeBarcode:
//   - it tries many rows of the frame (middle first, then up and down),
//   - each row is read normally and also reversed (upside-down barcode),
//   - first with the global threshold (toBinary), then with the local
//     threshold (toBinaryAdaptive) that copes with shadows.
// The check digit decides: a result is accepted only when fromEan13 agrees.
//
// For a good result the barcode should be at least about 2.5 pixels per
// module (so roughly 300 pixels wide with its white margin in a 640 pixel
// frame), flat, and with white margin on both sides inside the frame.
//
// Throws BarcodeError when no barcode can be read. The Increment 1 decoder
// is not changed or called by this function.
std::string decodeFrame(const Image& frame);

#endif // BARCODE_CAMERA_DECODER_HPP