#include "pos/storage/FileOrderDetailRepository.hpp"

namespace pos {

FileOrderDetailRepository::FileOrderDetailRepository(const std::string& path) : file_(path) {}

void FileOrderDetailRepository::add(const std::string& orderId, const OrderLine& line) {
    file_.append(OrderDetailCodec::toTokens(orderId, line));
}

List<OrderDetailRecord> FileOrderDetailRepository::all() const {
    List<List<std::string>> records = file_.readAll();
    List<OrderDetailRecord> rows;
    for (List<List<std::string>>::size_type i = 0; i < records.size(); ++i) {
        rows.push_back(OrderDetailCodec::parse(records[i]));
    }
    return rows;
}

}  // namespace pos
