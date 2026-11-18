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
    // Iterates through all possible start guard positions rather than giving up
    // on the first 1-0-1 pattern, to prevent background clutter or text to the left
    // from causing a false start guard failure.
    string readRuns(const vector<int>& runs) {
        if (runs.size() < 59) {
            return "";
        }

        const double fineTuningFactors[] = { 1.0, 0.98, 1.02, 0.96, 1.04, 0.94, 1.06 };

        // Search through all odd positions (black runs)
        for (int i = 1; i + 58 < static_cast<int>(runs.size()); i += 2) {
            double width = (runs[i] + runs[i + 1] + runs[i + 2]) / 3.0;
            if (width < 1.0) {
                continue;
            }

            // Quiet zone check: must have sufficient white space before the start guard
            if (runs[i - 1] < 1.2 * width) {
                continue;
            }

            // Start guard similarity check (runs i, i+1, i+2 must each be close to module width)
            bool similar = true;
            for (int k = 0; k < 3; k++) {
                if (runs[i + k] < 0.5 * width || runs[i + k] > 1.5 * width) {
                    similar = false;
                    break;
                }
            }
            if (!similar) {
                continue;
            }

            // Candidate start guard found!
            // Strategy 1: Average module width across the full standard 59-run (95-module) span.
            // If the barcode is uncorrupted, the end guard is at runs[i+56..i+58].
            long long totalPixels = 0;
            for (int k = 0; k < 59; ++k) {
                totalPixels += runs[i + k];
            }
            double globalAvgWidth = totalPixels / 95.0;
            if (globalAvgWidth >= 1.0) {
                try {
                    string ean13 = decodeRuns(runs, i, globalAvgWidth);
                    if (!ean13.empty()) {
                        return fromEan13(ean13);
                    }
                } catch (const BarcodeError&) {}
            }

            // Strategy 2: Use estimated start guard width with fine-tuning factors
            // to counteract slight lens distortion, camera sensor grid, or threshold dilation.
            for (double factor : fineTuningFactors) {
                try {
                    string ean13 = decodeRuns(runs, i, width * factor);
                    if (!ean13.empty()) {
                        return fromEan13(ean13);
                    }
                } catch (const BarcodeError&) {}
            }
        }

        return "";
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

    // Tries the rows of one binary image.
    // Concentrates scans around the center guide area (step of 4 pixels),
    // and steps outwards with a slightly larger step (8 pixels).
    string searchRows(const Image& binary) {
        const int height = binary.height;
        const int centre = height / 2;
        const int top = height * 10 / 100;
        const int bottom = height - 1 - top;

        // Phase 1: High-density scan inside the central guide area (30% to 70% height)
        // Stepping by 4 pixels ensures tilted barcodes and small barcodes are always hit.
        const int innerHalfHeight = height * 20 / 100; // ±20% around centre (y from 30% to 70%)
        const int innerStep = 4;

        for (int offset = 0; offset <= innerHalfHeight; offset += innerStep) {
            int rowDown = centre + offset;
            if (rowDown >= top && rowDown <= bottom) {
                string result = readRow(binary, rowDown);
                if (!result.empty()) {
                    return result;
                }
            }
            if (offset > 0) {
                int rowUp = centre - offset;
                if (rowUp >= top && rowUp <= bottom) {
                    string result = readRow(binary, rowUp);
                    if (!result.empty()) {
                        return result;
                    }
                }
            }
        }

        // Phase 2: Moderate-density scan in the outer regions
        const int outerStep = 8;
        for (int offset = innerHalfHeight + outerStep; ; offset += outerStep) {
            int rowDown = centre + offset;
            int rowUp = centre - offset;
            bool downOk = (rowDown >= top && rowDown <= bottom);
            bool upOk = (rowUp >= top && rowUp <= bottom);
            if (!downOk && !upOk) {
                break;
            }
            if (downOk) {
                string result = readRow(binary, rowDown);
                if (!result.empty()) {
                    return result;
                }
            }
            if (upOk) {
                string result = readRow(binary, rowUp);
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