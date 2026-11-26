// FileOrderRepository.cpp                                Owner: Person 2
// Implementation of FileOrderRepository for Order.txt.

#include "pos/storage/FileOrderRepository.hpp"

#include "pos/common/PosError.hpp"
#include "pos/common/StringUtils.hpp"
#include "pos/storage/OrderCodec.hpp"

namespace pos {

    FileOrderRepository::FileOrderRepository(const std::string& path) : file_(path) {}

    void FileOrderRepository::add(const Order& order) {
        file_.append(OrderCodec::toTokens(order));
    }

    std::optional<Order> FileOrderRepository::findById(const std::string& id) const {
        std::vector<Order> orders = all();
        for (const auto& ord : orders) {
            if (ord.id == id) {
                return ord;
            }
        }
        return std::nullopt;
    }

    std::vector<Order> FileOrderRepository::all() const {
        RecordSet records = file_.readAll();
        std::vector<Order> orders;
        for (const auto& rec : records) {
            if (!rec.empty()) {
                orders.push_back(OrderCodec::parse(rec));
            }
        }
        return orders;
    }

    void FileOrderRepository::update(const Order& order) {
        std::vector<Order> orders = all();
        bool found = false;
        for (auto& o : orders) {
            if (o.id == order.id) {
                o = order;
                found = true;
                break;
            }
        }
        if (!found) {
            throw PosError("Order with ID '" + order.id + "' not found to update");
        }

        List<List<std::string>> records;
        for (const auto& o : orders) {
            records.push_back(OrderCodec::toTokens(o));
        }
        file_.rewrite(records);
    }

    std::string FileOrderRepository::lastId() const {
        RecordSet records = file_.readAll();
        if (records.empty()) return std::string();

        const std::string& id = records.back()[0];
        if (id.size() != 8 || !isDigits(id)) {
            throw PosError("The last line of " + file_.path() + " does not start with an "
                "8-digit order id: '" + id + "'");
        }
        return id;
    }

}  // namespace pos