#ifndef BARCODE_PATTERNS_HPP
#define BARCODE_PATTERNS_HPP

#include <string>

std::string getLPattern(int digit);
std::string getRPattern(int digit);
int matchDigit(const std::string& modules, bool leftSide);

#endif