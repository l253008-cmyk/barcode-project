#include "pos/domain/Bill.hpp"
#include <cmath>

bool operator==(const BillLine& a, const BillLine& b) {
    return a.acronym == b.acronym &&
        a.unitPrice == b.unitPrice &&
        a.discount == b.discount &&
        std::fabs(a.quantity - b.quantity) < 1e-9;
}

bool operator!=(const BillLine& a, const BillLine& b) {
    return !(a == b);
}

bool operator==(const Bill& a, const Bill& b) {
    return a.id == b.id &&
        a.customerName == b.customerName &&
        a.orderId == b.orderId &&
        a.lines == b.lines &&
        a.totalWithoutTax == b.totalWithoutTax &&
        a.totalTax == b.totalTax &&
        a.totalDiscount == b.totalDiscount &&
        a.totalPayable == b.totalPayable &&
        a.status == b.status;
}

bool operator!=(const Bill& a, const Bill& b) {
    return !(a == b);
}