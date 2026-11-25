// CheckoutService.cpp                                 Owner: Person 1
// Implements checkout completion.

#include "pos/services/CheckoutService.hpp"

CheckoutService::CheckoutService(OrderService& orders,
    BillingService& billing,
    IBillRepository& bills,
    SeasonService* seasons)
    : orders_(orders), billing_(billing), bills_(&bills), seasons_(seasons) {
}

CheckoutService::CheckoutService(OrderService& orders, BillingService& billing)
    : orders_(orders), billing_(billing), bills_(nullptr), seasons_(nullptr) {
}

Bill CheckoutService::checkout() {
    return checkout(0.0, Money());
}

Bill CheckoutService::checkout(double specialPercent, Money amountReceived) {
    bool wasEditing = orders_.isEditingExisting();
    Order order = wasEditing ? orders_.finalizeEdit() : orders_.finalize();

    double seasonPercent = 0.0;
    if (seasons_) {
        auto opt = seasons_->percentFor(order.date);
        if (opt.has_value()) {
            seasonPercent = *opt;
        }
    }

    BillInputs inputs;
    inputs.date = order.date;
    inputs.time = order.time;
    inputs.seasonPercent = seasonPercent;
    inputs.specialPercent = specialPercent;
    inputs.amountReceived = amountReceived;

    Bill bill = billing_.createBill(order, inputs);

    if (bills_) {
        if (wasEditing) {
            auto existing = bills_->findByOrderId(order.id);
            if (existing.has_value()) {
                bill.id = existing->id;
                bills_->replace(bill);
            }
            else {
                bills_->add(bill);
            }
        }
        else {
            bills_->add(bill);
        }
    }

    return bill;
}

Bill CheckoutService::previewBill(const Order& order, double specialPercent, Money amountReceived) const {
    double seasonPercent = 0.0;
    if (seasons_) {
        auto opt = seasons_->percentFor(order.date);
        if (opt.has_value()) {
            seasonPercent = *opt;
        }
    }

    BillInputs inputs;
    inputs.billId = "PREVIEW";
    inputs.date = order.date;
    inputs.time = order.time;
    inputs.seasonPercent = seasonPercent;
    inputs.specialPercent = specialPercent;
    inputs.amountReceived = amountReceived;

    return billing_.createBill(order, inputs);
}