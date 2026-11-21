#ifndef POS_PRODUCT_HPP
#define POS_PRODUCT_HPP

#include <string>
#include <utility>
#include "pos/common/Money.hpp"

// a product is sold either by count (packets) or by weight (kg)
enum class MeasurementUnit { Count, Weight };

// one product, same fields as one line of Product.txt
struct Product {
    int id = 0;
    std::string categoryId;
    std::string barcode;
    std::string acronym;       // short name, max 8 letters
    std::string name;
    MeasurementUnit unit = MeasurementUnit::Count;
    Money unitPrice;
    Money tax;                 // per unit
    Money discount;            // per unit
    double availableQty = 0;   // how much is left in stock
    std::string barcodePath;

    Product() = default;

    // 10-field constructor for backward compatibility
    Product(int id_, std::string barcode_, std::string acronym_, std::string name_,
            MeasurementUnit unit_, Money unitPrice_, Money tax_, Money discount_,
            double availableQty_, std::string barcodePath_)
        : id(id_),
          categoryId(""),
          barcode(std::move(barcode_)),
          acronym(std::move(acronym_)),
          name(std::move(name_)),
          unit(unit_),
          unitPrice(unitPrice_),
          tax(tax_),
          discount(discount_),
          availableQty(availableQty_),
          barcodePath(std::move(barcodePath_)) {}

    // 11-field constructor for Increment 3
    Product(int id_, std::string categoryId_, std::string barcode_, std::string acronym_,
            std::string name_, MeasurementUnit unit_, Money unitPrice_, Money tax_,
            Money discount_, double availableQty_, std::string barcodePath_)
        : id(id_),
          categoryId(std::move(categoryId_)),
          barcode(std::move(barcode_)),
          acronym(std::move(acronym_)),
          name(std::move(name_)),
          unit(unit_),
          unitPrice(unitPrice_),
          tax(tax_),
          discount(discount_),
          availableQty(availableQty_),
          barcodePath(std::move(barcodePath_)) {}
};

inline bool operator==(const Product& a, const Product& b) {
    return a.id == b.id &&
           a.categoryId == b.categoryId &&
           a.barcode == b.barcode &&
           a.acronym == b.acronym &&
           a.name == b.name &&
           a.unit == b.unit &&
           a.unitPrice == b.unitPrice &&
           a.tax == b.tax &&
           a.discount == b.discount &&
           a.availableQty == b.availableQty &&
           a.barcodePath == b.barcodePath;
}

inline bool operator!=(const Product& a, const Product& b) {
    return !(a == b);
}

// unit to text and back ("count" or "weight")
std::string unitToText(MeasurementUnit unit);
MeasurementUnit unitFromText(const std::string& text);

// quantity to text without extra zeros, 40 gives "40" and 25.5 gives "25.5"
std::string quantityToText(double quantity);

#endif