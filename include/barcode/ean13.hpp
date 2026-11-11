#ifndef BARCODE_EAN13_HPP
#define BARCODE_EAN13_HPP

#include <string>

int computeCheckDigit(const std::string& first12Digits);
std::string toEan13(const std::string& code);
std::string fromEan13(const std::string& ean13);
std::string buildModules(const std::string& ean13);

#endif