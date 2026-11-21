
#include "pos/storage/ProductCodec.hpp"
#include "barcode/validator.hpp"
#include "pos/common/PosError.hpp"

using namespace std;

// whole number like "12", anything else is an error
static int parseWholeNumber(const string& text, const string& fieldName) {
    int length = static_cast<int>(text.size());
    if (length == 0 || length > 9) {
        throw PosError(fieldName + " must be a whole number: '" + text + "'");
    }

    int number = 0;
    for (int i = 0; i < length; i++) {
        if (text[i] < '0' || text[i] > '9') {
            throw PosError(fieldName + " must be a whole number: '" + text + "'");
        }
        number = number * 10 + (text[i] - '0');
    }
    return number;
}

// money like "350.00", it cannot be negative
static Money parseMoneyField(const string& text, const string& fieldName) {
    Money amount;
    try {
        amount = Money::parse(text);
    }
    catch (const PosError&) {
        throw PosError(fieldName + " is not a valid amount: '" + text + "'");
    }

    if (amount.paisa() < 0) {
        throw PosError(fieldName + " cannot be negative: '" + text + "'");
    }
    return amount;
}

// quantity like "40" or "25.5", letters, minus or two dots are errors
static double parseQuantityField(const string& text) {
    int length = static_cast<int>(text.size());
    bool seenDot = false;
    int digitsBeforeDot = 0;
    int digitsAfterDot = 0;
    bool valid = (length > 0 && length <= 15);

    for (int i = 0; i < length && valid; i++) {
        if (text[i] == '.') {
            if (seenDot) {
                valid = false;
            }
            seenDot = true;
        }
        else if (text[i] >= '0' && text[i] <= '9') {
            if (seenDot) {
                digitsAfterDot++;
            }
            else {
                digitsBeforeDot++;
            }
        }
        else {
            valid = false;
        }
    }

    if (digitsBeforeDot == 0 || (seenDot && digitsAfterDot == 0)) {
        valid = false;
    }
    if (!valid) {
        throw PosError("Available quantity is not a number: '" + text + "'");
    }
    return stod(text);
}

// join tokens from firstIndex to lastIndex with one space between them
static string joinWithSpaces(const vector<string>& tokens, int firstIndex, int lastIndex) {
    string text;
    for (int i = firstIndex; i <= lastIndex; i++) {
        if (i > firstIndex) {
            text += " ";
        }
        text += tokens[i];
    }
    return text;
}

// a line looks like:
// Old (10 fields): id barcode acronym name unit price tax discount quantity path
// New (11 fields): id categoryId barcode acronym name unit price tax discount quantity path
// Backward compatibility: If tokens[1] has 6 digits, it's old format.
// If tokens[1] has 2 digits, it's new format.
Product ProductCodec::parse(const vector<string>& tokens) {
    int count = static_cast<int>(tokens.size());
    if (count < 10) {
        throw PosError("A product line needs at least 10 fields, found " + to_string(count));
    }

    int id = parseWholeNumber(tokens[0], "Product id");
    if (id < 1) {
        throw PosError("Product id must be 1 or more: '" + tokens[0] + "'");
    }

    string categoryId;
    string barcode;
    string acronym;
    string name;

    // Detect format based on token 1 length
    if (tokens[1].size() == 6) {
        // Old 10-field format
        barcode = tokens[1];
        categoryId = "";
        acronym = tokens[2];
        name = joinWithSpaces(tokens, 3, count - 7);
    } else {
        // New 11-field format (needs at least 11 tokens)
        if (count < 11) {
            throw PosError("A 11-field product line needs at least 11 tokens, found " + to_string(count));
        }
        categoryId = tokens[1];
        if (categoryId.size() != 2 || categoryId[0] < '1' || categoryId[0] > '9' || categoryId[1] < '0' || categoryId[1] > '9') {
            throw PosError("Category ID must be 2 digits (10 to 99): '" + categoryId + "'");
        }
        barcode = tokens[2];
        acronym = tokens[3];
        name = joinWithSpaces(tokens, 4, count - 7);
    }

    if (!isValidCode(barcode)) {
        throw PosError("Product barcode is not a valid code: '" + barcode + "'");
    }

    if (acronym.size() > 8) {
        throw PosError("Product acronym is longer than 8 characters: '" + acronym + "'");
    }

    // from the right (last 6 tokens)
    MeasurementUnit unit = unitFromText(tokens[count - 6]);
    Money unitPrice = parseMoneyField(tokens[count - 5], "Unit price");
    Money tax = parseMoneyField(tokens[count - 4], "Tax");
    Money discount = parseMoneyField(tokens[count - 3], "Discount");
    double availableQty = parseQuantityField(tokens[count - 2]);
    string barcodePath = tokens[count - 1];

    // half a packet cannot be in stock
    double wholePart = static_cast<double>(static_cast<long long>(availableQty));
    if (unit == MeasurementUnit::Count && wholePart != availableQty) {
        throw PosError("A count product needs a whole available quantity: '" + tokens[count - 2] + "'");
    }

    Product product = { id, categoryId, barcode, acronym, name, unit,
                        unitPrice, tax, discount, availableQty, barcodePath };
    return product;
}

vector<string> ProductCodec::format(const Product& product) {
    vector<string> tokens;

    tokens.push_back(to_string(product.id));
    if (!product.categoryId.empty()) {
        tokens.push_back(product.categoryId);
    }
    tokens.push_back(product.barcode);
    tokens.push_back(product.acronym);
    tokens.push_back(product.name);                 // spaces inside the name are fine
    tokens.push_back(unitToText(product.unit));
    tokens.push_back(product.unitPrice.toString());
    tokens.push_back(product.tax.toString());
    tokens.push_back(product.discount.toString());
    tokens.push_back(quantityToText(product.availableQty));
    tokens.push_back(product.barcodePath);

    return tokens;
}
