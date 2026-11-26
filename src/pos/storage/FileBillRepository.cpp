// FileBillRepository.cpp                             Owner: Person 2
// Implementation of FileBillRepository for Bill.txt and BillDetail.txt.

#include "pos/storage/FileBillRepository.hpp"
#include "pos/common/PosError.hpp"
#include "pos/common/TextRecordFile.hpp"
#include "pos/storage/BillCodec.hpp"
#include "pos/storage/BillDetailCodec.hpp"

#include <filesystem>
#include <map>

using namespace std;

static string deriveDetailPath(const string& billPath) {
    filesystem::path p(billPath);
    return (p.parent_path() / "BillDetail.txt").string();
}

FileBillRepository::FileBillRepository(const string& billPath, const string& detailPath)
    : billPath_(billPath), detailPath_(detailPath) {
}

FileBillRepository::FileBillRepository(const string& billPath)
    : billPath_(billPath), detailPath_(deriveDetailPath(billPath)) {
}

void FileBillRepository::add(const Bill& bill) {
    // 1. Check for duplicate bill id in Bill.txt
    TextRecordFile billFile(billPath_);
    vector<vector<string>> existingBillRecords = billFile.readAll();
    for (const auto& rec : existingBillRecords) {
        if (!rec.empty() && rec[0] == bill.id) {
            throw PosError("Bill ID '" + bill.id + "' already exists in " + billPath_);
        }
    }

    // 2. Write detail lines first to BillDetail.txt
    TextRecordFile detailFile(detailPath_);
    for (const auto& line : bill.lines) {
        BillDetailLine toWrite = line;
        toWrite.billId = bill.id;
        detailFile.append(BillDetailCodec::format(toWrite));
    }

    // 3. Write master bill line last to Bill.txt
    billFile.append(BillCodec::format(bill));
}

void FileBillRepository::replace(const Bill& bill) {
    // 1. Replace master line in Bill.txt
    TextRecordFile billFile(billPath_);
    vector<vector<string>> billRecords = billFile.readAll();
    bool found = false;
    for (auto& rec : billRecords) {
        if (!rec.empty() && rec[0] == bill.id) {
            rec = BillCodec::format(bill);
            found = true;
            break;
        }
    }
    if (!found) {
        throw PosError("Bill with ID '" + bill.id + "' not found to replace in " + billPath_);
    }
    billFile.rewrite(billRecords);

    // 2. Replace detail lines in BillDetail.txt
    TextRecordFile detailFile(detailPath_);
    vector<vector<string>> detailRecords = detailFile.readAll();
    vector<vector<string>> remainingDetails;
    for (const auto& rec : detailRecords) {
        if (!rec.empty() && rec[0] != bill.id) {
            remainingDetails.push_back(rec);
        }
    }
    for (const auto& line : bill.lines) {
        BillDetailLine toWrite = line;
        toWrite.billId = bill.id;
        remainingDetails.push_back(BillDetailCodec::format(toWrite));
    }
    detailFile.rewrite(remainingDetails);
}

optional<Bill> FileBillRepository::findByOrderId(const string& orderId) const {
    vector<Bill> bills = all();
    for (const auto& b : bills) {
        if (b.orderId == orderId) {
            return b;
        }
    }
    return nullopt;
}

vector<Bill> FileBillRepository::all() const {
    TextRecordFile billFile(billPath_);
    vector<vector<string>> billRecords = billFile.readAll();
    vector<Bill> bills;
    map<string, size_t> billIndexMap;

    for (const auto& rec : billRecords) {
        if (!rec.empty()) {
            Bill b = BillCodec::parse(rec);
            billIndexMap[b.id] = bills.size();
            bills.push_back(b);
        }
    }

    // Load detail lines from BillDetail.txt and join to bills
    TextRecordFile detailFile(detailPath_);
    vector<vector<string>> detailRecords = detailFile.readAll();
    for (const auto& rec : detailRecords) {
        if (!rec.empty()) {
            BillDetailLine line = BillDetailCodec::parse(rec);
            auto it = billIndexMap.find(line.billId);
            if (it != billIndexMap.end()) {
                bills[it->second].lines.push_back(line);
            }
        }
    }

    return bills;
}

string FileBillRepository::lastId() const {
    TextRecordFile billFile(billPath_);
    vector<vector<string>> records = billFile.readAll();
    if (records.empty() || records.back().empty()) {
        return "";
    }
    return records.back().front();
}