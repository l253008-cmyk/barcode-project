#ifndef POS_STORAGE_FILEORDERREPOSITORY_HPP
#define POS_STORAGE_FILEORDERREPOSITORY_HPP

#include <string>

#include "pos/common/List.hpp"
#include "pos/common/TextRecordFile.hpp"
#include "pos/storage/IOrderRepository.hpp"

namespace pos {

// Order.txt: one line per order, appended in the order they are made.
class FileOrderRepository : public IOrderRepository {
public:
    explicit FileOrderRepository(const std::string& path);

    void add(const Order& order) override;
    std::string lastId() const override;

    // Every stored order (without lines). Handy for tests and reports.
    List<Order> all() const;

private:
    TextRecordFile file_;
};

}  // namespace pos

using FileOrderRepository = pos::FileOrderRepository;

#endif
