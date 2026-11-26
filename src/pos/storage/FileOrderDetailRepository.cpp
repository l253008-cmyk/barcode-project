// FileOrderDetailRepository.cpp                          Owner: Person 2
// Implementation of FileOrderDetailRepository for OrderDetail.txt.

#include "pos/storage/FileOrderDetailRepository.hpp"

namespace pos {

    FileOrderDetailRepository::FileOrderDetailRepository(const std::string& path) : file_(path) {}

    void FileOrderDetailRepository::add(const std::string& orderId, const OrderLine& line) {
        file_.append(OrderDetailCodec::toTokens(orderId, line));
    }

    std::vector<OrderLine> FileOrderDetailRepository::findByOrderId(const std::string& orderId) const {
        RecordSet records = file_.readAll();
        std::vector<OrderLine> lines;
        for (const auto& rec : records) {
            if (!rec.empty() && rec[0] == orderId) {
                OrderDetailRecord r = OrderDetailCodec::parse(rec);
                OrderLine line;
                line.productId = r.productId;
                line.barcode = r.barcode;
                line.quantity = r.quantity;
                lines.push_back(line);
            }
        }
        return lines;
    }

    void FileOrderDetailRepository::replaceForOrder(const std::string& orderId, const std::vector<OrderLine>& lines) {
        RecordSet records = file_.readAll();
        List<List<std::string>> remaining;

        // Keep all lines from other orders
        for (const auto& rec : records) {
            if (!rec.empty() && rec[0] != orderId) {
                remaining.push_back(rec);
            }
        }

        // Append the replacement lines for this order
        for (const auto& line : lines) {
            remaining.push_back(OrderDetailCodec::toTokens(orderId, line));
        }

        file_.rewrite(remaining);
    }

    std::vector<IOrderDetailRepository::Entry> FileOrderDetailRepository::all() const {
        RecordSet records = file_.readAll();
        std::vector<Entry> rows;
        for (const auto& rec : records) {
            if (!rec.empty()) {
                OrderDetailRecord r = OrderDetailCodec::parse(rec);
                OrderLine line;
                line.productId = r.productId;
                line.barcode = r.barcode;
                line.quantity = r.quantity;
                rows.push_back(Entry{ r.orderId, line });
            }
        }
        return rows;
    }

}  // namespace pos