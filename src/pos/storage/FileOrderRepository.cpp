#include "pos/storage/FileOrderRepository.hpp"

#include "pos/common/PosError.hpp"
#include "pos/common/StringUtils.hpp"
#include "pos/storage/OrderCodec.hpp"

namespace pos {

FileOrderRepository::FileOrderRepository(const std::string& path) : file_(path) {}

void FileOrderRepository::add(const Order& order) {
    file_.append(OrderCodec::toTokens(order));
}

std::string FileOrderRepository::lastId() const {
    List<List<std::string>> records = file_.readAll();
    if (records.empty()) return std::string();

    const std::string& id = records.back()[0];
    if (id.size() != 8 || !isDigits(id)) {
        throw PosError("The last line of " + file_.path() + " does not start with an "
                       "8-digit order id: '" + id + "'");
    }
    return id;
}

List<Order> FileOrderRepository::all() const {
    List<List<std::string>> records = file_.readAll();
    List<Order> orders;
    for (List<List<std::string>>::size_type i = 0; i < records.size(); ++i) {
        orders.push_back(OrderCodec::parse(records[i]));
    }
    return orders;
}

}  // namespace pos
