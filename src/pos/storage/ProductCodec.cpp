
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
// id barcode acronym name unit price tax discount quantity path
// the name can have spaces, so we take 3 fields from the left and 6 from
// the right, and what is left in the middle is the name
Product ProductCodec::parse(const vector<string>& tokens) {
    // 3 + at least one word of name + 6 = 10
    int count = static_cast<int>(tokens.size());
    if (count < 10) {
        throw PosError("A product line needs at least 10 fields, found " + to_string(count));
    }

    // from the left
    int id = parseWholeNumber(tokens[0], "Product id");
    if (id < 1) {
        throw PosError("Product id must be 1 or more: '" + tokens[0] + "'");
    }

    string barcode = tokens[1];
    if (!isValidCode(barcode)) {
        throw PosError("Product barcode is not a valid code: '" + barcode + "'");
    }

    string acronym = tokens[2];
    if (acronym.size() > 8) {
        throw PosError("Product acronym is longer than 8 characters: '" + acronym + "'");
    }

    // from the right
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

    // the middle part is the name
    string name = joinWithSpaces(tokens, 3, count - 7);

    Product product = { id, barcode, acronym, name, unit,
                       unitPrice, tax, discount, availableQty, barcodePath };
    return product;
}

vector<string> ProductCodec::format(const Product& product) {
    vector<string> tokens;

    tokens.insert(tokens.end(), 1, to_string(product.id));
    tokens.insert(tokens.end(), 1, product.barcode);
    tokens.insert(tokens.end(), 1, product.acronym);
    tokens.insert(tokens.end(), 1, product.name);                 // spaces inside the name are fine
    tokens.insert(tokens.end(), 1, unitToText(product.unit));
    tokens.insert(tokens.end(), 1, product.unitPrice.toString());
    tokens.insert(tokens.end(), 1, product.tax.toString());
    tokens.insert(tokens.end(), 1, product.discount.toString());
    tokens.insert(tokens.end(), 1, quantityToText(product.availableQty));
    tokens.insert(tokens.end(), 1, product.barcodePath);

    return tokens;
}
