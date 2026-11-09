#pragma once
#include <string>

// These functions give the barcode pattern of a digit.
std::string getLPattern(int digit);
std::string getRPattern(int digit);
// Finds the digit that matches the given pattern.
int matchDigit(const std::string& modules, bool leftSide);