#include "barcode/scanner.hpp"

#include "barcode/patterns.hpp"

std::vector<int> scanRow(const Image& binaryImg, int row) {
    std::vector<int> runs;
    if (row < 0 || row >= binaryImg.height) {
        return runs;
    }

    bool currentIsBlack = false;   // we always start by counting white
    int length = 0;

    for (int x = 0; x < binaryImg.width; x++) {
        bool pixelIsBlack = binaryImg.pixels[row * binaryImg.width + x] < 128;

        if (pixelIsBlack == currentIsBlack) {
            length++;                       // same colour, run gets longer
        }
        else {
            runs.push_back(length);         // colour changed, save the run
            currentIsBlack = pixelIsBlack;
            length = 1;
        }
    }
    runs.push_back(length);                 // save the last run
    return runs;
}

bool findStartGuard(const std::vector<int>& runs, int& startIndex, double& moduleWidth) {
    // Black runs are at odd positions, so we only check those.
    for (int i = 1; i + 2 < (int)runs.size(); i += 2) {
        // The three runs black, white, black should all be about the same width.
        double width = (runs[i] + runs[i + 1] + runs[i + 2]) / 3.0;

        bool similar = true;
        for (int k = 0; k < 3; k++) {
            if (runs[i + k] < 0.5 * width || runs[i + k] > 1.5 * width) {
                similar = false;
            }
        }

        // A wide white gap (the quiet zone) must come before the guard.
        bool quietZoneBefore = runs[i - 1] >= 2 * width;

        if (width > 0 && similar && quietZoneBefore) {
            startIndex = i;
            moduleWidth = width;
            return true;
        }
    }
    return false;
}

std::string decodeRuns(const std::vector<int>& runs, int startIndex, double moduleWidth) {
    // Step 1: turn the run lengths back into 0s and 1s (one character per module).
    std::string modules = "";
    for (int j = startIndex; j < (int)runs.size() && (int)modules.size() < 95; j++) {
        int count = (int)(runs[j] / moduleWidth + 0.5);   // round to whole modules
        if (count < 1) {
            count = 1;
        }
        char colour = (j % 2 == 1) ? '1' : '0';           // odd = black
        modules.append(count, colour);
    }
    if ((int)modules.size() < 95) {
        return "";
    }
    modules = modules.substr(0, 95);

    // Step 2: the start, centre and end guards must be correct.
    if (modules.substr(0, 3) != "101" ||
        modules.substr(45, 5) != "01010" ||
        modules.substr(92, 3) != "101") {
        return "";
    }

    // Step 3: decode the 12 digits. EAN-13 does not draw the first digit,
    // and our padding makes it 0, so we add it ourselves.
    std::string digits = "0";
    for (int d = 0; d < 12; d++) {
        int start;
        bool leftSide;
        if (d < 6) {
            start = 3 + d * 7;           // left digits come right after the start guard
            leftSide = true;
        }
        else {
            start = 50 + (d - 6) * 7;    // right digits come after the centre guard
            leftSide = false;
        }

        int digit = matchDigit(modules.substr(start, 7), leftSide);
        if (digit < 0) {
            return "";
        }
        digits += (char)('0' + digit);
    }
    return digits;
}