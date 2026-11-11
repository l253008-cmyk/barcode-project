// test_ean13.cpp                                     Owner: Person 2
// Tests for patterns.cpp and ean13.cpp.
// assert stops the program if a check is false.

#include <cassert>
#include <iostream>
#include "barcode/ean13.hpp"
#include "barcode/error.hpp"
#include "barcode/patterns.hpp"
using namespace std;

// These helpers return true when the function throws a BarcodeError.

static bool patternThrows(int digit) {
    try {
        getLPattern(digit);
    }
    catch (const BarcodeError&) {
        return true;
    }
    return false;
}

static bool checkDigitThrows(const string& text) {
    try {
        computeCheckDigit(text);
    }
    catch (const BarcodeError&) {
        return true;
    }
    return false;
}

static bool toEan13Throws(const string& text) {
    try {
        toEan13(text);
    }
    catch (const BarcodeError&) {
        return true;
    }
    return false;
}

static bool fromEan13Throws(const string& text) {
    try {
        fromEan13(text);
    }
    catch (const BarcodeError&) {
        return true;
    }
    return false;
}

static bool buildModulesThrows(const string& text) {
    try {
        buildModules(text);
    }
    catch (const BarcodeError&) {
        return true;
    }
    return false;
}

int main() {
    // ---------- patterns ----------
    assert(getLPattern(0) == "0001101");
    assert(getLPattern(9) == "0001011");
    assert(getRPattern(0) == "1110010");
    assert(patternThrows(-1));
    assert(patternThrows(10));

    // Each digit is found from its own pattern, but not on the wrong side.
    for (int d = 0; d < 10; d++) {
        assert(matchDigit(getLPattern(d), true) == d);
        assert(matchDigit(getRPattern(d), false) == d);
        assert(matchDigit(getLPattern(d), false) == -1);
        assert(matchDigit(getRPattern(d), true) == -1);
    }

    // Bad patterns give -1.
    assert(matchDigit("", true) == -1);
    assert(matchDigit("000110", true) == -1);      // too short
    assert(matchDigit("00011011", true) == -1);    // too long
    assert(matchDigit("abcdefg", true) == -1);     // not 0 and 1
    assert(matchDigit("1111111", true) == -1);     // no such digit

    // ---------- computeCheckDigit ----------
    assert(computeCheckDigit("400638133393") == 1);
    assert(computeCheckDigit("000000123456") == 5);
    assert(computeCheckDigit("000000000000") == 0);
    assert(checkDigitThrows(""));
    assert(checkDigitThrows("12345678901"));       // 11 digits
    assert(checkDigitThrows("1234567890123"));     // 13 digits
    assert(checkDigitThrows("12345678901a"));      // a letter

    // ---------- toEan13 ----------
    assert(toEan13("123456") == "0000001234565");
    assert(toEan13("100000") == "0000001000009");
    assert(toEan13Throws(""));
    assert(toEan13Throws("12345"));                // too short
    assert(toEan13Throws("1234567"));              // too long
    assert(toEan13Throws("12a456"));               // a letter
    assert(toEan13Throws("12345 "));               // a space
    assert(toEan13Throws("000000"));               // starts with 0
    assert(toEan13Throws("012345"));               // starts with 0

    // ---------- fromEan13 ----------
    assert(fromEan13("0000001234565") == "123456");
    assert(fromEan13("0000000000000") == "000000");
    assert(fromEan13Throws(""));
    assert(fromEan13Throws("000000123456"));       // 12 digits
    assert(fromEan13Throws("00000012345655"));     // 14 digits
    assert(fromEan13Throws("000000123456a"));      // a letter
    assert(fromEan13Throws("0000001234566"));      // wrong check digit
    assert(fromEan13Throws("1000001234565"));      // padding is not zero

    // ---------- toEan13 then fromEan13 gives the same code ----------
    assert(fromEan13(toEan13("100000")) == "100000");
    assert(fromEan13(toEan13("123456")) == "123456");
    assert(fromEan13(toEan13("999999")) == "999999");

    // ---------- buildModules ----------
    // Expected string for 123456, built piece by piece.
    string goldenModules = "101";                  // start guard
    goldenModules += "0001101";                    // 0
    goldenModules += "0001101";                    // 0
    goldenModules += "0001101";                    // 0
    goldenModules += "0001101";                    // 0
    goldenModules += "0001101";                    // 0
    goldenModules += "0011001";                    // 1
    goldenModules += "01010";                      // centre guard
    goldenModules += "1101100";                    // 2 (right pattern)
    goldenModules += "1000010";                    // 3
    goldenModules += "1011100";                    // 4
    goldenModules += "1001110";                    // 5
    goldenModules += "1010000";                    // 6
    goldenModules += "1001110";                    // 5 (check digit)
    goldenModules += "101";                        // end guard

    string modules = buildModules("0000001234565");
    assert(modules.size() == 95);
    assert(modules.substr(0, 3) == "101");
    assert(modules.substr(92, 3) == "101");
    assert(modules.substr(45, 5) == "01010");
    assert(modules == goldenModules);

    assert(buildModulesThrows(""));
    assert(buildModulesThrows("000000123456"));    // 12 digits
    assert(buildModulesThrows("000000123456a"));   // a letter
    assert(buildModulesThrows("1000001234565"));   // first digit is not 0
    assert(buildModulesThrows("0000001234566"));   // wrong check digit

    cout << "test_ean13: all passed" << endl;
    return 0;
}