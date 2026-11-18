// CheckoutService.hpp                                 Owner: Person 1
// The one call the GUI makes to finish a sale.
// Finalizes the draft order (which reduces stock and persists order + details),
// generates the bill using BillingService, saves it via IBillRepository, and returns it.

#ifndef POS_CHECKOUT_SERVICE_HPP
#define POS_CHECKOUT_SERVICE_HPP

#include "pos/domain/Bill.hpp"
#include "pos/services/BillingService.hpp"
#include "pos/services/OrderService.hpp"
#include "pos/storage/IBillRepository.hpp"

class CheckoutService {
public:
    CheckoutService(OrderService& orders, BillingService& billing, IBillRepository& bills);
    CheckoutService(OrderService& orders, BillingService& billing);

    // Finalizes the current draft order, creates and saves the bill, and returns it.
    Bill checkout();

private:
    OrderService& orders_;
    BillingService& billing_;
    IBillRepository* bills_ = nullptr;
};

#endif  // POS_CHECKOUT_SERVICE_HPP
