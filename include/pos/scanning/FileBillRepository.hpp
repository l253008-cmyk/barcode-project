#ifndef POS_FILE_BILL_REPOSITORY_HPP
#define POS_FILE_BILL_REPOSITORY_HPP

#include "pos/common/TextRecordFile.hpp"
#include "pos/storage/IBillRepository.hpp"
#include <string>
#include <vector>

// Stores bills in Bill.txt. The line format is the job of BillCodec and the
// reading and writing of lines is the job of TextRecordFile.
class FileBillRepository : public IBillRepository {
public:
    explicit FileBillRepository(TextRecordFile file);

    void add(const Bill& bill) override;
    std::string lastId() override;

    // Every saved bill (used by tests and later by a bill history screen).
    std::vector<Bill> all() const;

private:
    TextRecordFile file_;
};

#endif // POS_FILE_BILL_REPOSITORY_HPP