// FileCategoryRepository.cpp                         Owner: Person 2
// Implementation of FileCategoryRepository for PCategory.txt.

#include "pos/storage/FileCategoryRepository.hpp"
#include "pos/common/PosError.hpp"
#include "pos/common/TextRecordFile.hpp"
#include "pos/storage/CategoryCodec.hpp"

using namespace std;

FileCategoryRepository::FileCategoryRepository(const string& path)
    : path_(path) {
}

vector<ProductCategory> FileCategoryRepository::all() const {
    vector<ProductCategory> list;
    TextRecordFile file(path_);
    vector<vector<string>> records = file.readAll();
    for (const auto& rec : records) {
        if (!rec.empty()) {
            list.push_back(CategoryCodec::parse(rec));
        }
    }
    return list;
}

optional<ProductCategory> FileCategoryRepository::findById(const string& id) const {
    vector<ProductCategory> list = all();
    for (const auto& cat : list) {
        if (cat.id == id) {
            return cat;
        }
    }
    return nullopt;
}

void FileCategoryRepository::add(const ProductCategory& category) {
    vector<ProductCategory> current = all();
    for (const auto& existing : current) {
        if (existing.id == category.id) {
            throw PosError("Category with ID " + category.id + " already exists");
        }
        if (existing.acronym == category.acronym) {
            throw PosError("Category with acronym '" + category.acronym + "' already exists");
        }
    }

    TextRecordFile file(path_);
    file.append(CategoryCodec::format(category));
}