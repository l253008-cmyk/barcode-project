// patterns.cpp                                     
// This file holds the bar patterns for the digits 0 to 9.
// Every digit is drawn with 7 "modules" (thin strips).
// A '1' means a black strip and a '0' means a white strip.
// The patterns come from the EAN-13 standard, so they must stay exactly
// as they are written here.

#include "barcode/patterns.hpp"
#include "barcode/error.hpp"

// The LEFT pattern of each digit.
// kLeft[0] is for digit 0, kLeft[1] is for digit 1, and so on.
// "static" means only this file can see this table.
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

// Gives the left pattern of a digit.
// Example: getLPattern(0) gives "0001101".
// The table has only 10 entries, so any other digit is an error.
string getLPattern(int digit) {
    if (digit < 0 || digit > 9) {
        throw BarcodeError("getLPattern: digit must be between 0 and 9");
    }
    return kLeft[digit];
}

// Gives the right pattern of a digit.
// The right pattern is the left pattern turned upside down in colour:
// every 0 becomes 1 and every 1 becomes 0.
// Example: getRPattern(0) gives "1110010".
string getRPattern(int digit) {
    string pattern = getLPattern(digit);      // If the digit is wrong, getLPattern throws the error for us.
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

// It gets 7 characters and finds which digit they belong to.
// Gives -1 when nothing matches. It never throws, because a wrong
// pattern is normal when the decoder reads a noisy picture.
int matchDigit(const string& modules, bool leftSide) {
    // A pattern must have exactly 7 characters.
    if (modules.size() != 7) {
        return -1;
    }
    for (int d = 0; d < 10; d++) {    // Try every digit from 0 to 9 and compare its pattern.
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