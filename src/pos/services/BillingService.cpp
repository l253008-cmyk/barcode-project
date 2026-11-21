
#include "pos/services/BillingService.hpp"
#include "pos/common/PosError.hpp"

BillingService::BillingService(std::function<std::string()> nextBillId)
    : nextBillId_(nextBillId) {
}

Bill BillingService::createBill(const Order& order) {
    if (order.lines.empty()) {
        throw PosError("An order without items cannot be billed");
    }
    Bill bill;
    Money totalWithoutTax;
    Money totalTax;
    Money totalDiscount;
    for (const OrderLine& item : order.lines) {
        if (item.quantity <= 0) {
            throw PosError("Quantity of " + item.acronym + " must be more than zero");
        }
        totalWithoutTax += item.unitPrice * item.quantity;
        totalTax += item.tax * item.quantity;
        totalDiscount += item.discount * item.quantity;
        BillLine line;
        line.acronym = item.acronym;
        line.unitPrice = item.unitPrice;
        line.discount = item.discount;
        line.quantity = item.quantity;
        bill.lines.push_back(line);
    }
    bill.id = nextBillId_();
    bill.customerName = order.customerName;
    bill.orderId = order.id;
    bill.totalWithoutTax = totalWithoutTax;
    bill.totalTax = totalTax;
    bill.totalDiscount = totalDiscount;
    bill.totalPayable = totalWithoutTax + totalTax - totalDiscount;
    bill.status = toString(order.status);
    return bill;
}
