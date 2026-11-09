#pragma once
#include <string>

// Calculates the last digit of an EAN-13 barcode.
int computeCheckDigit(const std::string& first12);
// Converts a 6-digit code into a 13-digit EAN-13 code.
std::string toEan13(const std::string& code);
// Converts a valid EAN-13 code back into a 6-digit code.
std::string fromEan13(const std::string& ean13);
// Builds the 95 barcode modules.
std::string buildModules(const std::string& ean13);