#ifndef POS_DOMAIN_ORDER_HPP
#define POS_DOMAIN_ORDER_HPP

#include <string>
#include <vector>

#include "pos/common/List.hpp"
#include "pos/common/Money.hpp"

namespace pos {

enum class OrderStatus { InProgress, OnHold, Paid, Archived };
enum class PaymentMode { Cash, Card, Wallet };

// One product line of an order. The price, tax and discount are copies taken at
// the moment of sale, so editing Product.txt later never changes an old order.
struct OrderLine {
    int productId = 0;
    std::string barcode, acronym;
    Money unitPrice, tax, discount;
    double quantity = 0.0;
};

struct Order {
    std::string id, date, time, customerName, contact;
    OrderStatus status = OrderStatus::InProgress;
    Money payment;
    PaymentMode mode = PaymentMode::Cash;
    std::string merchant;
    std::vector<OrderLine> lines;
};

// Text used in Order.txt: in-progress, on-hold, paid, archived / cash, card, wallet.
std::string toString(OrderStatus status);
std::string toString(PaymentMode mode);

// The other direction. Throws PosError for text that is not one of the names above.
OrderStatus orderStatusFromString(const std::string& text);
PaymentMode paymentModeFromString(const std::string& text);

}  // namespace pos

using Order = pos::Order;
using OrderLine = pos::OrderLine;
using OrderStatus = pos::OrderStatus;
using PaymentMode = pos::PaymentMode;
using pos::toString;
using pos::orderStatusFromString;
using pos::paymentModeFromString;

#endif
