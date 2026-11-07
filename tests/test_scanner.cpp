#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "barcode/scanner.hpp"

// The golden module string for code 123456 (95 characters).
const std::string GOLDEN =
"101"
"0001101" "0001101" "0001101" "0001101" "0001101" "0011001"
"01010"
"1101100" "1000010" "1011100" "1001110" "1010000" "1001110"
"101";

// Makes a one-row image from letters: B = black, W = white.
Image makeRowImage(const std::string& letters) {
    Image img;
    img.width = (int)letters.size();
    img.height = 1;
    for (int i = 0; i < img.width; i++) {
        img.pixels.push_back(letters[i] == 'B' ? 0 : 255);
    }
    return img;
}

// Builds run lengths from a module string.
// scale = pixels per module, quiet = white modules before and after.
std::vector<int> makeRuns(const std::string& modules, int scale, int quiet) {
    std::vector<int> runs;
    runs.push_back(quiet * scale);             // white quiet zone first
    int count = 1;
    for (int i = 1; i < (int)modules.size(); i++) {
        if (modules[i] == modules[i - 1]) {
            count++;
        }
        else {
            runs.push_back(count * scale);
            count = 1;
        }
    }
    runs.push_back(count * scale);
    runs.push_back(quiet * scale);             // white quiet zone last
    return runs;
}

void testScanRow() {
    // WWWBBBWWWWWW should give runs 3, 3, 6
    std::vector<int> runs = scanRow(makeRowImage("WWWBBBWWWWWW"), 0);
    assert(runs.size() == 3);
    assert(runs[0] == 3 && runs[1] == 3 && runs[2] == 6);

    // A row that starts with black gets a first white run of 0
    runs = scanRow(makeRowImage("BBWW"), 0);
    assert(runs.size() == 3);
    assert(runs[0] == 0 && runs[1] == 2 && runs[2] == 2);

    // A row number outside the image gives an empty list
    runs = scanRow(makeRowImage("BBWW"), 5);
    assert(runs.empty());
}

void testFindStartGuard() {
    std::vector<int> runs = makeRuns(GOLDEN, 3, 10);
    int startIndex = -1;
    double moduleWidth = 0;
    assert(findStartGuard(runs, startIndex, moduleWidth) == true);
    assert(startIndex == 1);
    assert(moduleWidth > 2.9 && moduleWidth < 3.1);

    // A blank (all white) image has no guard
    std::vector<int> blank = scanRow(makeRowImage("WWWWWWWWWWWW"), 0);
    assert(findStartGuard(blank, startIndex, moduleWidth) == false);
}

void testDecodeRuns() {
    std::vector<int> runs = makeRuns(GOLDEN, 3, 10);
    assert(decodeRuns(runs, 1, 3.0) == "0000001234565");

    // Tolerance: make one run 1 pixel too wide, it must still decode
    runs[5] = runs[5] + 1;
    assert(decodeRuns(runs, 1, 3.0) == "0000001234565");

    // Garbage input must give an empty string
    std::vector<int> bad = { 10, 3, 3, 3, 3 };
    assert(decodeRuns(bad, 1, 3.0) == "");
}

int main() {
    testScanRow();
    testFindStartGuard();
    testDecodeRuns();
    std::cout << "All scanner tests passed." << std::endl;
    return 0;
}