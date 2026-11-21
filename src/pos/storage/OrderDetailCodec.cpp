#include "pos/storage/OrderDetailCodec.hpp"

#include "pos/common/PosError.hpp"
#include "pos/common/StringUtils.hpp"

namespace pos {

namespace {

void checkFields(const std::string& orderId, long long productId,
                 const std::string& barcode, double quantity) {
    if (orderId.size() != 8 || !isDigits(orderId)) {
        throw PosError("Order id must be exactly 8 digits, got '" + orderId + "'");
    }
    if (productId <= 0) {
        throw PosError("Product id must be a positive number in order " + orderId);
    }
    if (!isDigits(barcode)) {
        throw PosError("Barcode must be digits only in order " + orderId + ", got '" + barcode + "'");
    }
    if (!(quantity > 0.0)) {
        throw PosError("Quantity must be greater than zero in order " + orderId);
    }
}

}  // namespace

OrderDetailRecord OrderDetailCodec::parse(const List<std::string>& tokens) {
    if (tokens.size() != 4) {
        throw PosError("An order detail record needs exactly 4 fields, found " +
                       std::to_string(tokens.size()));
    }

    long long productId = 0;
    if (!parseInt(tokens[1], productId)) {
        throw PosError("Product id is not a number: '" + tokens[1] + "'");
    }
    double quantity = 0.0;
    if (!parseDouble(tokens[3], quantity)) {
        throw PosError("Quantity is not a number: '" + tokens[3] + "'");
    }
    checkFields(tokens[0], productId, tokens[2], quantity);

    OrderDetailRecord record;
    record.orderId = tokens[0];
    record.productId = static_cast<int>(productId);
    record.barcode = tokens[2];
    record.quantity = quantity;
    return record;
}

OrderDetailRecord OrderDetailCodec::parseLine(const std::string& line) {
    return parse(split(stripEndMarker(line), ' '));
}

List<std::string> OrderDetailCodec::toTokens(const std::string& orderId, const OrderLine& line) {
    checkFields(orderId, line.productId, line.barcode, line.quantity);

    List<std::string> tokens;
    tokens.push_back(orderId);
    tokens.push_back(std::to_string(line.productId));
    tokens.push_back(line.barcode);
    tokens.push_back(formatQuantity(line.quantity));
    return tokens;
}

std::string OrderDetailCodec::format(const std::string& orderId, const OrderLine& line) {
    return join(toTokens(orderId, line), " ") + "#";
}

}  // namespace pos
