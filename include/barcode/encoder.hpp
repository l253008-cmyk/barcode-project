#ifndef BARCODE_ENCODER_HPP
#define BARCODE_ENCODER_HPP

#include <string>
#include "barcode/image.hpp"

Image generateBarcode(const std::string& code);
void generateToFile(const std::string& code, const std::string& path);

#endif