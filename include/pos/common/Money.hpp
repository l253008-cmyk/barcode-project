#ifndef POS_MONEY_HPP
#define POS_MONEY_HPP

#include "pos/common/PosError.hpp"
#include <cmath>
#include <string>

// Money kept as a whole number of paisa (1 PKR = 100 paisa).
// A whole number has no rounding errors, unlike double (0.1 + 0.2 is not 0.3).
// Header only: every function is small and inline.
class Money {
public:
    Money() : paisa_(0) {}

    static Money fromPaisa(long long paisa) {
        Money money;
        money.paisa_ = paisa;
        return money;
    }

    static Money fromDouble(double amount) {
        return fromPaisa(std::llround(amount * 100.0));
    }

    // Reads text such as "350", "350.0" or "350.00" (also with a leading minus).
    // Throws PosError for anything else, so a bad record is never accepted.
    static Money parse(const std::string& text) {
        std::size_t i = 0;
        bool negative = false;
        if (i < text.size() && text[i] == '-') {
            negative = true;
            i++;
        }

        long long whole = 0;
        int wholeDigits = 0;
        while (i < text.size() && text[i] >= '0' && text[i] <= '9') {
            whole = whole * 10 + (text[i] - '0');
            wholeDigits++;
            i++;
            if (wholeDigits > 15) {
                throw PosError("Money amount is too large: '" + text + "'");
            }
        }
        if (wholeDigits == 0) {
            throw PosError("Not a money amount: '" + text + "'");
        }

        long long fraction = 0;
        int fractionDigits = 0;
        if (i < text.size() && text[i] == '.') {
            i++;
            while (i < text.size() && text[i] >= '0' && text[i] <= '9') {
                fraction = fraction * 10 + (text[i] - '0');
                fractionDigits++;
                i++;
            }
            if (fractionDigits == 0 || fractionDigits > 2) {
                throw PosError("Money needs 1 or 2 digits after the dot: '" + text + "'");
            }
        }
        if (i != text.size()) {
            throw PosError("Not a money amount: '" + text + "'");
        }
        if (fractionDigits == 1) {
            fraction = fraction * 10;   // "17.5" means 17.50
        }

        long long paisa = whole * 100 + fraction;
        return fromPaisa(negative ? -paisa : paisa);
    }

    long long paisa() const { return paisa_; }

    // Always two digits after the dot: 35000 paisa becomes "350.00".
    std::string toString() const {
        bool negative = paisa_ < 0;
        unsigned long long magnitude = negative
            ? 0ULL - static_cast<unsigned long long>(paisa_)
            : static_cast<unsigned long long>(paisa_);

        std::string cents = std::to_string(magnitude % 100);
        if (cents.size() < 2) {
            cents = "0" + cents;
        }
        return std::string(negative ? "-" : "") + std::to_string(magnitude / 100) + "." + cents;
    }

    Money operator+(const Money& other) const { return fromPaisa(paisa_ + other.paisa_); }
    Money operator-(const Money& other) const { return fromPaisa(paisa_ - other.paisa_); }
    Money& operator+=(const Money& other) {
        paisa_ += other.paisa_;
        return *this;
    }

    // Price x quantity, rounded to the nearest paisa (quantity may be 1.5 kg).
    Money operator*(double quantity) const {
        return fromPaisa(std::llround(static_cast<double>(paisa_) * quantity));
    }

    bool operator==(const Money& other) const { return paisa_ == other.paisa_; }
    bool operator!=(const Money& other) const { return paisa_ != other.paisa_; }

private:
    long long paisa_;
};

#endif // POS_MONEY_HPP