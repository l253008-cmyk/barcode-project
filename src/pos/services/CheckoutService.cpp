// CheckoutService.cpp                                 Owner: Person 1
// Implements checkout completion.

#include "pos/services/CheckoutService.hpp"

CheckoutService::CheckoutService(OrderService& orders, BillingService& billing, IBillRepository& bills)
    : orders_(orders), billing_(billing), bills_(&bills) {}

CheckoutService::CheckoutService(OrderService& orders, BillingService& billing)
    : orders_(orders), billing_(billing), bills_(nullptr) {}

Bill CheckoutService::checkout() {
    Order order = orders_.finalize();
    Bill bill = billing_.createBill(order);
    if (bills_) {
        bills_->add(bill);
    }
    return bill;
}
