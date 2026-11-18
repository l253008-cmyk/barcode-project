#ifndef POS_STORAGE_FILEORDERDETAILREPOSITORY_HPP
#define POS_STORAGE_FILEORDERDETAILREPOSITORY_HPP

#include <string>

#include "pos/common/List.hpp"
#include "pos/common/TextRecordFile.hpp"
#include "pos/storage/IOrderDetailRepository.hpp"
#include "pos/storage/OrderDetailCodec.hpp"

namespace pos {

// OrderDetail.txt: one line per product line of every order.
class FileOrderDetailRepository : public IOrderDetailRepository {
public:
    explicit FileOrderDetailRepository(const std::string& path);

    void add(const std::string& orderId, const OrderLine& line) override;

    // Every stored row. Handy for tests and reports.
    List<OrderDetailRecord> all() const;

private:
    TextRecordFile file_;
};

}  // namespace pos

#endif
