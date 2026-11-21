// FileOrderRepository.hpp                                Owner: Person 2
// File-backed implementation of IOrderRepository for Order.txt.

#ifndef POS_STORAGE_FILEORDERREPOSITORY_HPP
#define POS_STORAGE_FILEORDERREPOSITORY_HPP

#include <optional>
#include <string>
#include <vector>

#include "pos/common/TextRecordFile.hpp"
#include "pos/storage/IOrderRepository.hpp"

namespace pos {

class FileOrderRepository : public IOrderRepository {
public:
    explicit FileOrderRepository(const std::string& path);

    void add(const Order& order) override;
    std::optional<Order> findById(const std::string& id) const override;
    std::vector<Order> all() const override;
    void update(const Order& order) override;
    std::string lastId() const override;

private:
    TextRecordFile file_;
};

}  // namespace pos

using FileOrderRepository = pos::FileOrderRepository;

#endif // POS_STORAGE_FILEORDERREPOSITORY_HPP
