// FileSeasonRepository.cpp                           Owner: Person 2
// Implementation of FileSeasonRepository for Seasons.txt.

#include "pos/storage/FileSeasonRepository.hpp"
#include "pos/common/PosError.hpp"
#include "pos/common/TextRecordFile.hpp"
#include "pos/storage/SeasonCodec.hpp"

using namespace std;

FileSeasonRepository::FileSeasonRepository(const string& path)
    : path_(path) {
}

vector<SeasonDiscount> FileSeasonRepository::all() const {
    vector<SeasonDiscount> list;
    TextRecordFile file(path_);
    vector<vector<string>> records = file.readAll();
    for (const auto& rec : records) {
        if (!rec.empty()) {
            list.push_back(SeasonCodec::parse(rec));
        }
    }
    return list;
}

void FileSeasonRepository::add(const SeasonDiscount& season) {
    TextRecordFile file(path_);
    file.append(SeasonCodec::format(season));
}

void FileSeasonRepository::setActive(size_t index, bool flag) {
    vector<SeasonDiscount> list = all();
    if (index >= list.size()) {
        throw PosError("Invalid season index: " + to_string(index));
    }
    list[index].active = flag;

    vector<vector<string>> records;
    for (const auto& s : list) {
        records.push_back(SeasonCodec::format(s));
    }

    TextRecordFile file(path_);
    file.rewrite(records);
}
