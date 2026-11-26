// FileOrderDetailRepository.hpp                          Owner: Person 2
// File-backed implementation of IOrderDetailRepository for OrderDetail.txt.

#ifndef POS_STORAGE_FILEORDERDETAILREPOSITORY_HPP
#define POS_STORAGE_FILEORDERDETAILREPOSITORY_HPP

#include <string>
#include <vector>

#include "pos/common/TextRecordFile.hpp"
#include "pos/storage/IOrderDetailRepository.hpp"
#include "pos/storage/OrderDetailCodec.hpp"

namespace pos {

    class FileOrderDetailRepository : public IOrderDetailRepository {
    public:
        explicit FileOrderDetailRepository(const std::string& path);

        void add(const std::string& orderId, const OrderLine& line) override;
        std::vector<OrderLine> findByOrderId(const std::string& orderId) const override;
        void replaceForOrder(const std::string& orderId, const std::vector<OrderLine>& lines) override;
        std::vector<Entry> all() const override;

    private:
        TextRecordFile file_;
    };

}  // namespace pos

using FileOrderDetailRepository = pos::FileOrderDetailRepository;

#endif // POS_STORAGE_FILEORDERDETAILREPOSITORY_HPP