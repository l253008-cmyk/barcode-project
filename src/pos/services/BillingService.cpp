// BillingService.cpp                                  Owner: Person 4
// Implementation of BillingService calculation formula.

#include "pos/services/BillingService.hpp"
#include "pos/common/PosError.hpp"

#include <cmath>

using namespace std;

BillingService::BillingService(function<string()> nextBillId)
    : nextBillId_(nextBillId) {
}

Bill BillingService::createBill(const Order& order, const BillInputs& inputs) {
    if (order.lines.empty()) {
        throw PosError("Cannot create a bill for an order with no items");
    }

    if (inputs.specialPercent < 0.0 || inputs.specialPercent > 8.0) {
        throw PosError("Special discount percent cannot exceed 8% (given: " + to_string(inputs.specialPercent) + "%)");
    }
    if (inputs.seasonPercent < 0.0 || inputs.seasonPercent > 100.0) {
        throw PosError("Seasonal discount percent cannot exceed 100%");
    }

    string billId = inputs.billId;
    if (billId.empty() && nextBillId_) {
        billId = nextBillId_();
    }

    Money totalWithoutTax;
    Money totalTax;
    Money totalDiscount;
    vector<BillDetailLine> detailLines;

    for (const auto& line : order.lines) {
        if (!(line.quantity > 0.0)) {
            throw PosError("Item quantity must be greater than zero for product: " + line.acronym);
        }

        Money lineAmount = line.unitPrice * line.quantity;
        Money lineTax = line.tax * line.quantity;
        Money lineDiscount = line.discount * line.quantity;

        totalWithoutTax += lineAmount;
        totalTax += lineTax;
        totalDiscount += lineDiscount;

        BillDetailLine detail;
        detail.billId = billId;
        detail.acronym = line.acronym;
        detail.unitPrice = line.unitPrice;
        detail.quantity = line.quantity;
        detail.discount = lineDiscount;
        detailLines.push_back(detail);
    }

    // subtotal = totalWithoutTax + totalTax - totalDiscount
    Money subtotal = totalWithoutTax + totalTax - totalDiscount;

    // seasonalDiscount = subtotal * seasonPercent / 100
    Money seasonalDiscount;
    if (inputs.seasonPercent > 0.0) {
        double paisa = static_cast<double>(subtotal.paisa()) * (inputs.seasonPercent / 100.0);
        seasonalDiscount = Money::fromPaisa(llround(paisa));
    }

    // specialDiscount = (subtotal - seasonalDiscount) * specialPercent / 100
    Money specialDiscount;
    if (inputs.specialPercent > 0.0) {
        Money baseAfterSeason = subtotal - seasonalDiscount;
        double paisa = static_cast<double>(baseAfterSeason.paisa()) * (inputs.specialPercent / 100.0);
        specialDiscount = Money::fromPaisa(llround(paisa));
    }

    // totalPayable = subtotal - seasonalDiscount - specialDiscount
    Money totalPayable = subtotal - seasonalDiscount - specialDiscount;

    // If amountReceived is zero/unspecified, default it to totalPayable
    Money received = inputs.amountReceived;
    if (received.paisa() == 0) {
        received = totalPayable;
    }

    if (received.paisa() < totalPayable.paisa()) {
        throw PosError("Amount received (" + received.toString() + ") is less than total payable (" + totalPayable.toString() + ")");
    }

    Money balanceReturned = received - totalPayable;

    string customer = order.customerName.empty() ? "Walk-in" : order.customerName;
    string date = inputs.date.empty() ? order.date : inputs.date;
    string time = inputs.time.empty() ? order.time : inputs.time;

    Bill bill;
    bill.id = billId;
    bill.customerName = customer;
    bill.orderId = order.id;
    bill.totalWithoutTax = totalWithoutTax;
    bill.totalTax = totalTax;
    bill.totalDiscount = totalDiscount;
    bill.totalPayable = totalPayable;
    bill.amountReceived = received;
    bill.balanceReturned = balanceReturned;
    bill.status = "paid";
    bill.seasonalDiscount = seasonalDiscount;
    bill.specialDiscount = specialDiscount;
    bill.date = date;
    bill.time = time;
    bill.lines = detailLines;

    return bill;
}

Bill BillingService::createBill(const Order& order) {
    BillInputs inputs;
    inputs.date = order.date;
    inputs.time = order.time;
    return createBill(order, inputs);
}
