#ifndef POS_PRODUCT_HPP
#define POS_PRODUCT_HPP

#include <string>
#include "pos/common/Money.hpp"

// a product is sold either by count (packets) or by weight (kg)
enum class MeasurementUnit { Count, Weight };

// one product, same fields as one line of Product.txt
struct Product {
    int id;
    std::string barcode;
    std::string acronym;       // short name, max 8 letters
    std::string name;
    MeasurementUnit unit;
    Money unitPrice;
    Money tax;                 // per unit
    Money discount;            // per unit
    double availableQty;       // how much is left in stock
    std::string barcodePath;
};

// unit to text and back ("count" or "weight")
std::string unitToText(MeasurementUnit unit);
MeasurementUnit unitFromText(const std::string& text);

// quantity to text without extra zeros, 40 gives "40" and 25.5 gives "25.5"
std::string quantityToText(double quantity);

#endif