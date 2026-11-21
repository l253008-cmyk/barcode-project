#ifndef POS_COMMON_TEXTRECORDFILE_HPP
#define POS_COMMON_TEXTRECORDFILE_HPP

#include <string>
#include <vector>

#include "pos/common/List.hpp"
#include "pos/common/PosError.hpp"

namespace pos {

class RecordSet {
public:
    RecordSet(List<List<std::string>> records) : records_(std::move(records)) {}

    operator List<List<std::string>>() const { return records_; }

    operator std::vector<std::vector<std::string>>() const {
        std::vector<std::vector<std::string>> result;
        result.reserve(records_.size());
        for (std::string::size_type i = 0; i < records_.size(); ++i) {
            std::vector<std::string> row;
            row.reserve(records_[i].size());
            for (std::string::size_type j = 0; j < records_[i].size(); ++j) {
                row.push_back(records_[i][j]);
            }
            result.push_back(std::move(row));
        }
        return result;
    }

    using size_type = List<List<std::string>>::size_type;
    size_type size() const { return records_.size(); }
    bool empty() const { return records_.empty(); }
    List<std::string>& operator[](size_type i) { return records_[i]; }
    const List<std::string>& operator[](size_type i) const { return records_[i]; }
    const List<std::string>& back() const { return records_.back(); }
    List<std::string>& back() { return records_.back(); }
    auto begin() { return records_.begin(); }
    auto end() { return records_.end(); }
    auto begin() const { return records_.begin(); }
    auto end() const { return records_.end(); }

private:
    List<List<std::string>> records_;
};

// One data file (Order.txt, Product.txt ...). A record is one line: fields
// separated by single spaces and a '#' written straight after the last field.
// This class turns lines into token lists and back, and nothing else. What the
// tokens mean is the codec's job.
class TextRecordFile {
public:
    explicit TextRecordFile(const std::string& path);

    // Every record as a list of tokens, '#' removed, blank lines skipped.
    // Creates the file if it is missing. Throws PosError when a line has no
    // '#' at the end (a half-written line) or the file cannot be opened.
    RecordSet readAll() const;

    // Adds one record at the end. Tokens may contain spaces (a name) but not
    // '#', not a line break, and not be empty (use "-" for an empty field).
    void append(const List<std::string>& tokens);
    void append(const std::vector<std::string>& tokens);

    // Replaces the whole file. The new content goes to "<path>.tmp" first and
    // only replaces the real file when it was written completely.
    void rewrite(const List<List<std::string>>& records);
    void rewrite(const std::vector<std::vector<std::string>>& records);

    // Deletes the file from disk. Returns false if there was nothing to delete.
    // Meant for tests that clean up after themselves.
    bool removeFromDisk();

    const std::string& path() const { return path_; }

private:
    void ensureExists() const;
    bool endsWithNewline() const;

    std::string path_;
};

}  // namespace pos

using TextRecordFile = pos::TextRecordFile;

#endif
