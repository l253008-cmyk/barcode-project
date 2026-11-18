#include "pos/common/TextRecordFile.hpp"

#include "pos/common/StringUtils.hpp"

// Files cannot be opened with <iostream> and <string> alone, so <fstream> is
// needed here, and <cstdio> for deleting and renaming a file. This .cpp and
// SystemClock.cpp are the only places that use more than the two libraries.
#include <cstdio>
#include <fstream>
#include <iostream>

namespace pos {

namespace {

// A record must be writable and readable back as the same tokens.
void checkTokens(const List<std::string>& tokens, const std::string& path) {
    if (tokens.empty()) {
        throw PosError("Cannot write an empty record to " + path);
    }
    for (List<std::string>::size_type i = 0; i < tokens.size(); ++i) {
        const std::string& token = tokens[i];
        if (trim(token).empty()) {
            throw PosError("Field " + std::to_string(i + 1) + " is empty in a record for " +
                           path + " (write - for an empty field)");
        }
        if (token.find_first_of("#\r\n") != std::string::npos) {
            throw PosError("Field '" + token + "' contains # or a line break, "
                           "which the data files cannot store");
        }
    }
}

std::string toLine(const List<std::string>& tokens) {
    std::string line;
    for (List<std::string>::size_type i = 0; i < tokens.size(); ++i) {
        if (i > 0) line += ' ';
        line += trim(tokens[i]);
    }
    line += '#';
    return line;
}

}  // namespace

TextRecordFile::TextRecordFile(const std::string& path) : path_(path) {}

void TextRecordFile::ensureExists() const {
    if (path_.empty()) {
        throw PosError("No file path was given for a data file");
    }
    std::ifstream probe(path_.c_str());
    if (probe.is_open()) return;

    std::ofstream create(path_.c_str(), std::ios::app);
    if (!create.is_open()) {
        throw PosError("Cannot create the data file " + path_ +
                       " (does its folder exist?)");
    }
}

bool TextRecordFile::endsWithNewline() const {
    std::ifstream in(path_.c_str(), std::ios::binary);
    if (!in.is_open()) return true;
    in.seekg(0, std::ios::end);
    std::streamoff size = in.tellg();
    if (size <= 0) return true;  // empty file, nothing to separate from
    in.seekg(-1, std::ios::end);
    char last = '\0';
    in.get(last);
    return last == '\n';
}

List<List<std::string>> TextRecordFile::readAll() const {
    ensureExists();

    std::ifstream in(path_.c_str());
    if (!in.is_open()) {
        throw PosError("Cannot open the data file " + path_);
    }

    List<List<std::string>> records;
    std::string line;
    std::string::size_type lineNumber = 0;
    while (std::getline(in, line)) {
        ++lineNumber;
        if (lineNumber == 1 && line.compare(0, 3, "\xEF\xBB\xBF") == 0) {
            line.erase(0, 3);  // a byte order mark left by some editors
        }

        std::string text = trim(line);
        if (text.empty()) continue;

        if (text[text.size() - 1] != '#') {
            throw PosError(path_ + ", line " + std::to_string(lineNumber) +
                           ": the end marker # is missing (the line may be half written)");
        }
        List<std::string> tokens = split(stripEndMarker(text), ' ');
        if (tokens.empty()) {
            throw PosError(path_ + ", line " + std::to_string(lineNumber) +
                           ": the record has no fields");
        }
        records.push_back(tokens);
    }
    return records;
}

void TextRecordFile::append(const List<std::string>& tokens) {
    checkTokens(tokens, path_);
    ensureExists();

    // If someone edited the file by hand and left no final line break, start
    // on a fresh line so two records never run together.
    bool needsBreak = !endsWithNewline();

    std::ofstream out(path_.c_str(), std::ios::app);
    if (!out.is_open()) {
        throw PosError("Cannot open the data file for writing: " + path_);
    }
    if (needsBreak) out << '\n';
    out << toLine(tokens) << '\n';
    out.flush();
    if (!out) {
        throw PosError("Writing to " + path_ + " failed");
    }
}

void TextRecordFile::rewrite(const List<List<std::string>>& records) {
    for (List<List<std::string>>::size_type i = 0; i < records.size(); ++i) {
        checkTokens(records[i], path_);
    }
    if (path_.empty()) {
        throw PosError("No file path was given for a data file");
    }

    const std::string tempPath = path_ + ".tmp";
    {
        std::ofstream out(tempPath.c_str(), std::ios::trunc);
        if (!out.is_open()) {
            throw PosError("Cannot create the temporary file " + tempPath);
        }
        for (List<List<std::string>>::size_type i = 0; i < records.size(); ++i) {
            out << toLine(records[i]) << '\n';
        }
        out.flush();
        out.close();
        if (out.fail()) {
            std::remove(tempPath.c_str());
            throw PosError("Writing the temporary file " + tempPath + " failed; " +
                           path_ + " was not changed");
        }
    }

    // Windows will not rename over an existing file, so remove the old one
    // first. The complete new data is already safe in the temporary file.
    std::remove(path_.c_str());
    if (std::rename(tempPath.c_str(), path_.c_str()) != 0) {
        throw PosError("Could not replace " + path_ + ". The new data is saved in " +
                       tempPath);
    }
}

bool TextRecordFile::removeFromDisk() {
    return std::remove(path_.c_str()) == 0;
}

}  // namespace pos
