// ean13.cpp                                          

#include "barcode/ean13.hpp"
#include "barcode/config.hpp"
#include "barcode/error.hpp"
#include "barcode/patterns.hpp"
#include "barcode/validator.hpp"

// True if every character is a digit from 0 to 9.
static bool allDigits(const string& text) {
    int length = static_cast<int>(text.size());
    for (int i = 0; i < length; i++) {
        if (text[i] < '0' || text[i] > '9') {
            return false;
        }
    }
    return true;
}

// Check digit of the first 12 digits.
// Odd positions are multiplied by 1, even positions by 3.
int computeCheckDigit(const string& first12Digits) {
    if (first12Digits.size() != 12 || !allDigits(first12Digits)) {
        throw BarcodeError("computeCheckDigit: need exactly 12 digits");
    }
    int weightedSum = 0;
    for (int i = 0; i < 12; i++) {
        int digit = first12Digits[i] - '0';
        if (i % 2 == 0) {
            weightedSum += digit;
        }
        else {
            weightedSum += 3 * digit;
        }
    }
    int checkDigit = (10 - weightedSum % 10) % 10;  // The last "% 10" gives 0 instead of 10 when the sum ends in 0.
    return checkDigit;
}

// "123456" becomes "0000001234565" (6 zeros + code + check digit).
string toEan13(const string& code) {
    requireValidCode(code);
    string zeros = string(kPaddingDigits, '0');
    string first12Digits = zeros + code;
    int checkDigit = computeCheckDigit(first12Digits);
    string checkDigitText = to_string(checkDigit);
    string ean13 = first12Digits + checkDigitText;
    return ean13;
}

// "0000001234565" becomes "123456". Throws if the 13 digits are not valid.
string fromEan13(const string& ean13) {
    if (ean13.size() != 13 || !allDigits(ean13)) {
        throw BarcodeError("fromEan13: need exactly 13 digits");
    }
    string first12Digits = ean13.substr(0, 12);
    int correctCheckDigit = computeCheckDigit(first12Digits);
    int givenCheckDigit = ean13[12] - '0';
    if (givenCheckDigit != correctCheckDigit) {
        throw BarcodeError("fromEan13: check digit does not match");
    }
    for (int i = 0; i < kPaddingDigits; i++) {
        if (ean13[i] != '0') {
            throw BarcodeError("fromEan13: padding digits must be zero");
        }
    }

    string code = ean13.substr(kPaddingDigits, kCodeLength);
    return code;
}

// start guard + 6 left digits + centre guard + 6 right digits + end guard.
string buildModules(const string& ean13) {
    if (ean13.size() != 13 || !allDigits(ean13)) {
        throw BarcodeError("buildModules: need exactly 13 digits");
    }
    if (ean13[0] != '0') {
        throw BarcodeError("buildModules: first digit must be 0");  // The first digit is not drawn, so it must always be 0.
    }

    string first12Digits = ean13.substr(0, 12);
    int correctCheckDigit = computeCheckDigit(first12Digits);
    int givenCheckDigit = ean13[12] - '0';
    if (givenCheckDigit != correctCheckDigit) {
        throw BarcodeError("buildModules: check digit does not match");
    }

    string modules = "101";                          // start guard
    for (int i = 1; i <= 6; i++) {                   // left digits
        int digit = ean13[i] - '0';
        modules += getLPattern(digit);
    }
    modules += "01010";                              // centre guard
    for (int i = 7; i <= 12; i++) {                  // right digits
        int digit = ean13[i] - '0';
        modules += getRPattern(digit);
    }

    modules += "101";                                // end guard
    return modules;
}