#include "barcode/camera_decoder.hpp"
#include "barcode/adaptive.hpp"
#include "barcode/ean13.hpp"
#include "barcode/error.hpp"
#include "barcode/preprocess.hpp"
#include "barcode/scanner.hpp"
#include <string>
#include <vector>
using namespace std;

namespace {

    // Rows are tried from the middle outwards in steps of 1/kRowStepDivisor of the
    // frame height, but only inside the middle part of the frame (the first and
    // last kMarginPercent percent are often dark or blurred).
    const int kRowStepDivisor = 20;
    const int kMarginPercent = 10;

    // Turns the run lengths of one row into a 6-digit code.
    // Returns "" on any failure, so the caller can try the next row.
    // (The same steps as the private helpers of decoder.cpp, which cannot be
    // reused from here because they are static and Increment 1 stays unchanged.)
    string readRuns(const vector<int>& runs) {
        try {
            int startIndex = 0;
            double moduleWidth = 0;
            if (!findStartGuard(runs, startIndex, moduleWidth)) {
                return "";
            }
            string ean13 = decodeRuns(runs, startIndex, moduleWidth);
            if (ean13.empty()) {
                return "";
            }
            return fromEan13(ean13);   // verifies the check digit and the padding
        }
        catch (const BarcodeError&) {
            return "";
        }
    }

    // One row of the binary image: normal, then reversed.
    string readRow(const Image& binary, int row) {
        string result = readRuns(scanRow(binary, row));
        if (!result.empty()) {
            return result;
        }

        Image reversed;
        reversed.width = binary.width;
        reversed.height = 1;
        reversed.pixels.resize(binary.width);
        for (int x = 0; x < binary.width; x++) {
            reversed.at(x, 0) = binary.at(binary.width - 1 - x, row);
        }
        return readRuns(scanRow(reversed, 0));
    }

    // Tries the rows of one binary image. Returns "" when no row works.
    string searchRows(const Image& binary) {
        const int height = binary.height;
        const int centre = height / 2;
        int step = height / kRowStepDivisor;
        if (step < 1) {
            step = 1;
        }
        const int top = height * kMarginPercent / 100;
        const int bottom = height - 1 - top;

        for (int k = 0; ; k++) {
            const int below = centre + k * step;
            const int above = centre - k * step;
            const bool belowInside = below >= top && below <= bottom;
            const bool aboveInside = above >= top && above <= bottom;
            if (!belowInside && !aboveInside) {
                break;
            }

            if (belowInside) {
                string result = readRow(binary, below);
                if (!result.empty()) {
                    return result;
                }
            }
            if (k > 0 && aboveInside) {
                string result = readRow(binary, above);
                if (!result.empty()) {
                    return result;
                }
            }
        }
        return "";
    }

}  // namespace

string decodeFrame(const Image& frame) {
    // The global threshold is fast and enough for most frames.
    // (toBinary also refuses an empty or damaged frame with a BarcodeError.)
    string result = searchRows(toBinary(frame));
    if (!result.empty()) {
        return result;
    }

    // The local threshold copes with shadows and uneven light.
    result = searchRows(toBinaryAdaptive(frame));
    if (!result.empty()) {
        return result;
    }

    throw BarcodeError("No barcode found in the frame");
}