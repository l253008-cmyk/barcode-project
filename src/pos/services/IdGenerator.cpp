#include "pos/services/IdGenerator.hpp"

#include "pos/common/PosError.hpp"
#include "pos/common/StringUtils.hpp"

namespace pos {

namespace {

// Reads the running number out of the front of the last id.
long long lastRunningNumber(const std::string& lastId, std::string::size_type digits,
                            std::string::size_type totalLength, const char* what) {
    if (lastId.empty()) return 0;
    if (lastId.size() != totalLength || !isDigits(lastId)) {
        throw PosError(std::string("The last ") + what + " id is not " +
                       std::to_string(totalLength) + " digits: '" + lastId + "'");
    }
    long long number = 0;
    parseInt(lastId.substr(0, digits), number);
    return number;
}

long long largestNumber(std::string::size_type digits) {
    long long limit = 1;
    for (std::string::size_type i = 0; i < digits; ++i) limit *= 10;
    return limit - 1;
}

}  // namespace

IdGenerator::IdGenerator(IOrderRepository& orders, IBillRepository& bills, IClock& clock)
    : orders_(orders), bills_(bills), clock_(clock) {}

std::string IdGenerator::nextOrderId() {
    const std::string::size_type runningDigits = 6;
    long long next = lastRunningNumber(orders_.lastId(), runningDigits, 8, "order") + 1;
    if (next > largestNumber(runningDigits)) {
        throw PosError("Order numbers are used up (the running number has only 6 digits)");
    }

    std::string today = clock_.today();  // YYYY-MM-DD
    if (today.size() != 10 || today[4] != '-' || today[7] != '-' ||
        !isDigits(today.substr(8, 2))) {
        throw PosError("The clock gave a date in an unexpected form: '" + today + "'");
    }
    return padLeft(std::to_string(next), runningDigits, '0') + today.substr(8, 2);
}

std::string IdGenerator::nextBillId() {
    const std::string::size_type digits = 9;
    long long next = lastRunningNumber(bills_.lastId(), digits, digits, "bill") + 1;
    if (next > largestNumber(digits)) {
        throw PosError("Bill numbers are used up (the running number has only 9 digits)");
    }
    return padLeft(std::to_string(next), digits, '0');
}

}  // namespace pos
