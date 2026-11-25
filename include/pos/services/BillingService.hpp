// BillingService.hpp                                  Owner: Person 4
// Service calculating bills from orders with seasonal and special discounts.

#ifndef POS_BILLING_SERVICE_HPP
#define POS_BILLING_SERVICE_HPP

#include <functional>
#include <string>

#include "pos/domain/Bill.hpp"
#include "pos/domain/Order.hpp"

struct BillInputs {
    std::string billId;
    std::string date;
    std::string time;
    double seasonPercent = 0.0;
    double specialPercent = 0.0;
    Money amountReceived;
};

class BillingService {
public:
    explicit BillingService(std::function<std::string()> nextBillId);

    // Calculates a full bill with season discount and special discount (capped at 8%)
    Bill createBill(const Order& order, const BillInputs& inputs);

    // Default overload without extra discounts (amountReceived defaults to payable)
    Bill createBill(const Order& order);

private:
    std::function<std::string()> nextBillId_;
};

#endif // POS_BILLING_SERVICE_HPP