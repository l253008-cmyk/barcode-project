#ifndef POS_BILLING_SERVICE_HPP
#define POS_BILLING_SERVICE_HPP

#include "pos/domain/Bill.hpp"
#include "pos/domain/Order.hpp"
#include <functional>
#include <string>

// Calculates a bill from an order. It does not save the bill (the repository
// does that) and it does not show it (the GUI does that).
class BillingService {
public:
    // nextBillId gives a new 9-digit bill id each time it is called, for
    // example [&]() { return idGenerator.nextBillId(); }
    explicit BillingService(std::function<std::string()> nextBillId);

    // Line amount = unit price x quantity, tax = tax x quantity,
    // discount = discount x quantity (each rounded to a paisa, then added).
    // Total payable = total without tax + total tax - total discount.
    // Throws PosError for an order without items or with a quantity <= 0.
    Bill createBill(const Order& order);

private:
    std::function<std::string()> nextBillId_;
};

#endif // POS_BILLING_SERVICE_HPP