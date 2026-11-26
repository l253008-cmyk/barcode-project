// FileBillRepository.hpp                             Owner: Person 2
// File-backed implementation of IBillRepository for Bill.txt and BillDetail.txt.

#ifndef POS_FILE_BILL_REPOSITORY_HPP
#define POS_FILE_BILL_REPOSITORY_HPP

#include <optional>
#include <string>
#include <vector>

#include "pos/storage/IBillRepository.hpp"

class FileBillRepository : public IBillRepository {
public:
    explicit FileBillRepository(const std::string& billPath, const std::string& detailPath);
    explicit FileBillRepository(const std::string& billPath);

    void add(const Bill& bill) override;
    void replace(const Bill& bill) override;
    std::optional<Bill> findByOrderId(const std::string& orderId) const override;
    std::vector<Bill> all() const override;
    std::string lastId() const override;

    const std::string& billPath() const { return billPath_; }
    const std::string& detailPath() const { return detailPath_; }

private:
    std::string billPath_;
    std::string detailPath_;
};

#endif // POS_FILE_BILL_REPOSITORY_HPP