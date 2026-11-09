#ifndef BARCODE_VALIDATOR_HPP
#define BARCODE_VALIDATOR_HPP

#include <string>

// Checks whether the code is valid
bool isValidCode(const std::string& code);

// Checks the code and throws an error if it is invalid
void requireValidCode(const std::string& code);

#endif