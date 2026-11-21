// CheckoutService.hpp                                 Owner: Person 1
// The one call the GUI makes to finish a sale.
// Finalizes the draft order (or edited order),
// calculates seasonal and special discounts,
// generates the bill using BillingService, saves it via IBillRepository, and returns it.

#ifndef POS_CHECKOUT_SERVICE_HPP
#define POS_CHECKOUT_SERVICE_HPP

#include "pos/domain/Bill.hpp"
#include "pos/services/BillingService.hpp"
#include "pos/services/OrderService.hpp"
#include "pos/services/SeasonService.hpp"
#include "pos/storage/IBillRepository.hpp"

class CheckoutService {
public:
    CheckoutService(OrderService& orders,
                    BillingService& billing,
                    IBillRepository& bills,
                    SeasonService* seasons = nullptr);
    CheckoutService(OrderService& orders, BillingService& billing);

    // Finalizes the current draft order (or edited order), creates and saves the bill, and returns it.
    Bill checkout();

    // Overload taking payment details and discounts
    Bill checkout(double specialPercent, Money amountReceived);

private:
    OrderService& orders_;
    BillingService& billing_;
    IBillRepository* bills_ = nullptr;
    SeasonService* seasons_ = nullptr;
};

#endif // POS_CHECKOUT_SERVICE_HPP
