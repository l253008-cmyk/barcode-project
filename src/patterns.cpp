// patterns.cpp                                       Owner: Person 2
// Bar patterns for the digits 0 to 9 (EAN-13 standard).
// Each digit is 7 modules: '1' is a black strip, '0' is a white strip.

#include "barcode/patterns.hpp"
#include "barcode/error.hpp"

using namespace std;

// Left pattern of each digit. kLeft[3] is the pattern of digit 3.
static const char* kLeft[10] = {
    "0001101",   // 0
    "0011001",   // 1
    "0010011",   // 2
    "0111101",   // 3
    "0100011",   // 4
    "0110001",   // 5
    "0101111",   // 6
    "0111011",   // 7
    "0110111",   // 8
    "0001011"    // 9
};

// Left pattern of a digit. Example: 0 gives "0001101".
string getLPattern(int digit) {
    // The table has only 10 entries, so any other digit is an error.
    if (digit < 0 || digit > 9) {
        throw BarcodeError("getLPattern: digit must be between 0 and 9");
    }
    return kLeft[digit];
}

// Right pattern = left pattern with every 0 and 1 swapped.
// Example: 0 gives "1110010".
string getRPattern(int digit) {
    // A wrong digit makes getLPattern throw the error for us.
    string pattern = getLPattern(digit);

    int length = static_cast<int>(pattern.size());
    for (int i = 0; i < length; i++) {
        if (pattern[i] == '0') {
            pattern[i] = '1';
        }
        else {
            pattern[i] = '0';
        }
    }
    return pattern;
}

// Finds the digit that owns these 7 characters. Gives -1 if none does.
// leftSide = true looks in the left patterns, false in the right patterns.
// It never throws, because the decoder meets wrong patterns in noisy pictures.
int matchDigit(const string& modules, bool leftSide) {
    if (modules.size() != 7) {
        return -1;
    }
    for (int d = 0; d < 10; d++) {
        string pattern;
        if (leftSide) {
            pattern = getLPattern(d);
        }
        else {
            pattern = getRPattern(d);
        }

        if (modules == pattern) {
            return d;
        }
    }
    return -1;
}