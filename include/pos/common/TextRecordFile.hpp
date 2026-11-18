#ifndef POS_COMMON_TEXTRECORDFILE_HPP
#define POS_COMMON_TEXTRECORDFILE_HPP

#include <string>

#include "pos/common/List.hpp"
#include "pos/common/PosError.hpp"

namespace pos {

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
    List<List<std::string>> readAll() const;

    // Adds one record at the end. Tokens may contain spaces (a name) but not
    // '#', not a line break, and not be empty (use "-" for an empty field).
    void append(const List<std::string>& tokens);

    // Replaces the whole file. The new content goes to "<path>.tmp" first and
    // only replaces the real file when it was written completely.
    void rewrite(const List<List<std::string>>& records);

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

#endif
