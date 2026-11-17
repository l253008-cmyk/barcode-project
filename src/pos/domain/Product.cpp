#include "pos/domain/Product.hpp"
#include "pos/common/PosError.hpp"

using namespace std;

// Count gives "count", Weight gives "weight"
string unitToText(MeasurementUnit unit) {
    if (unit == MeasurementUnit::Weight) {
        return "weight";
    }
    return "count";
}

// "count" or "weight" back to the unit, any other word is an error
MeasurementUnit unitFromText(const string& text) {
    if (text == "count") {
        return MeasurementUnit::Count;
    }
    if (text == "weight") {
        return MeasurementUnit::Weight;
    }
    throw PosError("unitFromText: unit must be count or weight, not '" + text + "'");
}

// to_string adds extra zeros (25.500000), so remove them
string quantityToText(double quantity) {
    string text = to_string(quantity);
    while (!text.empty() && text.back() == '0') {
        text.pop_back();
    }
    if (!text.empty() && text.back() == '.') {
        text.pop_back();
    }
    return text;
}