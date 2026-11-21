#include "pos/storage/FileBillRepository.hpp"
#include "pos/storage/BillCodec.hpp"

FileBillRepository::FileBillRepository(TextRecordFile file)
    : file_(file) {
}

FileBillRepository::FileBillRepository(const std::string& path)
    : file_(path) {
}

void FileBillRepository::add(const Bill& bill) {
    // Make all records first, so a bad bill is refused before the file is touched.
    std::vector<std::vector<std::string>> records = BillCodec::toRecords(bill);
    for (const std::vector<std::string>& record : records) {
        file_.append(record);
    }
}

std::string FileBillRepository::lastId() const {
    std::vector<std::vector<std::string>> records = file_.readAll();
    if (records.empty() || records.back().empty()) {
        return "";
    }
    return records.back().front();   // the first token of a line is the bill id
}

std::vector<Bill> FileBillRepository::all() const {
    return BillCodec::fromRecords(file_.readAll());
}