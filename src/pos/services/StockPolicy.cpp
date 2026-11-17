#include "pos/services/StockPolicy.hpp"

using namespace std;

bool StockPolicy::canFulfil(const Product& product, double quantity) const {
    // zero or negative quantity is not allowed (this also catches not-a-number)
    if (!(quantity > 0)) {
        return false;
    }

    // cannot sell more than we have, selling the last unit is fine
    if (quantity > product.availableQty) {
        return false;
    }

    // count products must be whole numbers, 2 is ok but 1.5 is not
    if (product.unit == MeasurementUnit::Count) {
        double wholePart = static_cast<double>(static_cast<long long>(quantity));
        if (wholePart != quantity) {
            return false;
        }
    }

    return true;
}