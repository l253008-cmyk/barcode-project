#include "pos/domain/Order.hpp"

#include "pos/common/PosError.hpp"

namespace pos {

std::string toString(OrderStatus status) {
    switch (status) {
        case OrderStatus::InProgress: return "in-progress";
        case OrderStatus::OnHold:     return "on-hold";
        case OrderStatus::Paid:       return "paid";
        case OrderStatus::Archived:   return "archived";
    }
    throw PosError("Unknown order status value");
}

std::string toString(PaymentMode mode) {
    switch (mode) {
        case PaymentMode::Cash:   return "cash";
        case PaymentMode::Card:   return "card";
        case PaymentMode::Wallet: return "wallet";
    }
    throw PosError("Unknown payment mode value");
}

OrderStatus orderStatusFromString(const std::string& text) {
    if (text == "in-progress") return OrderStatus::InProgress;
    if (text == "on-hold")     return OrderStatus::OnHold;
    if (text == "paid")        return OrderStatus::Paid;
    if (text == "archived")    return OrderStatus::Archived;
    throw PosError("Unknown order status '" + text +
                   "' (expected in-progress, on-hold, paid or archived)");
}

PaymentMode paymentModeFromString(const std::string& text) {
    if (text == "cash")   return PaymentMode::Cash;
    if (text == "card")   return PaymentMode::Card;
    if (text == "wallet") return PaymentMode::Wallet;
    throw PosError("Unknown payment mode '" + text +
                   "' (expected cash, card or wallet)");
}

}  // namespace pos
