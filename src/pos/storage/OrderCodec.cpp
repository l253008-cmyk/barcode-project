#include "pos/storage/OrderCodec.hpp"

#include "pos/common/PosError.hpp"
#include "pos/common/StringUtils.hpp"

namespace pos {

namespace {

const char* const kEmpty = "-";
const List<std::string>::size_type kFixedBefore = 3;  // id, date, time
const List<std::string>::size_type kFixedAfter = 4;   // status, payment, mode, merchant
const List<std::string>::size_type kMinTokens = kFixedBefore + kFixedAfter + 2;  // name + contact

bool isTwoDigitRange(const std::string& text, long long low, long long high) {
    long long value = 0;
    return text.size() == 2 && isDigits(text) && parseInt(text, value) &&
           value >= low && value <= high;
}

bool isValidDate(const std::string& s) {
    return s.size() == 10 && s[4] == '-' && s[7] == '-' && isDigits(s.substr(0, 4)) &&
           isTwoDigitRange(s.substr(5, 2), 1, 12) && isTwoDigitRange(s.substr(8, 2), 1, 31);
}

bool isValidTime(const std::string& s) {
    return s.size() == 8 && s[2] == ':' && s[5] == ':' &&
           isTwoDigitRange(s.substr(0, 2), 0, 23) &&
           isTwoDigitRange(s.substr(3, 2), 0, 59) &&
           isTwoDigitRange(s.substr(6, 2), 0, 59);
}

bool isValidOrderId(const std::string& s) { return s.size() == 8 && isDigits(s); }

bool hasWhitespace(const std::string& s) {
    return s.find_first_of(" \t\r\n") != std::string::npos;
}

void checkHeader(const std::string& id, const std::string& date, const std::string& time) {
    if (!isValidOrderId(id)) {
        throw PosError("Order id must be exactly 8 digits, got '" + id + "'");
    }
    if (!isValidDate(date)) {
        throw PosError("Order date must look like 2026-11-14, got '" + date + "'");
    }
    if (!isValidTime(time)) {
        throw PosError("Order time must look like 10:32:05, got '" + time + "'");
    }
}

std::string emptyAsDash(const std::string& value) { return value.empty() ? kEmpty : value; }
std::string dashAsEmpty(const std::string& value) { return value == kEmpty ? std::string() : value; }

// A contact or merchant is one token, so it may not contain spaces.
std::string singleWord(const std::string& value, const char* what) {
    std::string cleaned = trim(value);
    if (hasWhitespace(cleaned)) {
        throw PosError(std::string("The ") + what + " must not contain spaces: '" + cleaned + "'");
    }
    return cleaned;
}

}  // namespace

Order OrderCodec::parse(const List<std::string>& tokens) {
    const List<std::string>::size_type n = tokens.size();
    if (n < kMinTokens) {
        throw PosError("An order record needs at least " + std::to_string(kMinTokens) +
                       " fields, found " + std::to_string(n));
    }

    Order order;
    order.id = tokens[0];
    order.date = tokens[1];
    order.time = tokens[2];
    checkHeader(order.id, order.date, order.time);

    order.status = orderStatusFromString(tokens[n - 4]);
    order.payment = Money::parse(tokens[n - 3]);
    order.mode = paymentModeFromString(tokens[n - 2]);
    order.merchant = dashAsEmpty(tokens[n - 1]);

    order.contact = dashAsEmpty(tokens[n - 5]);
    order.customerName = dashAsEmpty(joinRange(tokens, kFixedBefore, n - 5, " "));
    return order;
}

Order OrderCodec::parseLine(const std::string& line) {
    return parse(split(stripEndMarker(line), ' '));
}

List<std::string> OrderCodec::toTokens(const Order& order) {
    checkHeader(order.id, order.date, order.time);

    // Several spaces in a typed name become one, as they would on reading back.
    std::string name = join(split(order.customerName, ' '), " ");
    if (name.find('#') != std::string::npos) {
        throw PosError("The customer name must not contain #: '" + name + "'");
    }
    std::string contact = singleWord(order.contact, "contact");
    std::string merchant = singleWord(order.merchant, "merchant");

    List<std::string> tokens;
    tokens.push_back(order.id);
    tokens.push_back(order.date);
    tokens.push_back(order.time);
    tokens.push_back(emptyAsDash(name));
    tokens.push_back(emptyAsDash(contact));
    tokens.push_back(toString(order.status));
    tokens.push_back(order.payment.toString());
    tokens.push_back(toString(order.mode));
    tokens.push_back(emptyAsDash(merchant));
    return tokens;
}

std::string OrderCodec::format(const Order& order) {
    return join(toTokens(order), " ") + "#";
}

}  // namespace pos
