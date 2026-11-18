// Tests for TextRecordFile (P3). Run from any folder: it makes its own scratch
// files in the current folder and deletes them at the end.
#include <fstream>
#include <iostream>
#include <string>

#include "pos/common/PosError.hpp"
#include "pos/common/StringUtils.hpp"
#include "pos/common/TextRecordFile.hpp"

static int failures = 0;

#define CHECK(condition)                                                              \
    do {                                                                              \
        if (!(condition)) {                                                           \
            std::cout << "FAIL (line " << __LINE__ << "): " #condition << std::endl;  \
            ++failures;                                                               \
        }                                                                             \
    } while (0)

#define CHECK_THROWS(statement)                                                       \
    do {                                                                              \
        bool threw = false;                                                           \
        try { statement; } catch (const pos::PosError&) { threw = true; }             \
        if (!threw) {                                                                 \
            std::cout << "FAIL (line " << __LINE__ << "): no PosError from " #statement \
                      << std::endl;                                                   \
            ++failures;                                                               \
        }                                                                             \
    } while (0)

using pos::List;
using pos::TextRecordFile;

static const char* kPath = "scratch_text_record_file.txt";

static void writeRaw(const std::string& content) {
    std::ofstream out(kPath, std::ios::binary | std::ios::trunc);
    out << content;
}

static std::string readRaw(const std::string& path) {
    std::ifstream in(path.c_str(), std::ios::binary);
    std::string all, line;
    while (std::getline(in, line)) all += line + "\n";
    return all;
}

static List<std::string> record(const std::string& a, const std::string& b = "",
                                const std::string& c = "") {
    List<std::string> tokens;
    tokens.push_back(a);
    if (!b.empty()) tokens.push_back(b);
    if (!c.empty()) tokens.push_back(c);
    return tokens;
}

static void testMissingFileIsCreated() {
    TextRecordFile file(kPath);
    file.removeFromDisk();
    CHECK(file.readAll().empty());
    std::ifstream probe(kPath);
    CHECK(probe.is_open());  // readAll created it
}

static void testAppendAndReadBack() {
    TextRecordFile file(kPath);
    file.removeFromDisk();

    file.append(record("00012314", "2026-11-14", "paid"));
    file.append(record("Ali Raza", "0300-1234567"));  // a name with a space in one token

    CHECK(readRaw(kPath) == "00012314 2026-11-14 paid#\nAli Raza 0300-1234567#\n");

    List<List<std::string>> all = file.readAll();
    CHECK(all.size() == 2);
    CHECK(all[0].size() == 3);
    CHECK(all[0][0] == "00012314");
    CHECK(all[0][2] == "paid");   // '#' is gone
    CHECK(all[1].size() == 3);    // "Ali Raza" came back as two tokens
    CHECK(all[1][0] == "Ali" && all[1][1] == "Raza");
}

static void testReadingHandMadeFile() {
    // blank lines, a space before '#', CRLF line endings, a byte order mark
    writeRaw("\xEF\xBB\xBF" "1 100001 TEA 40 #\r\n\r\n   \r\n2 100002 SUGAR 25.5#\r\n");
    TextRecordFile file(kPath);
    List<List<std::string>> all = file.readAll();
    CHECK(all.size() == 2);
    CHECK(all[0].size() == 4 && all[0][3] == "40");
    CHECK(all[1].size() == 4 && all[1][3] == "25.5");
}

static void testMissingMarkerIsRejected() {
    writeRaw("1 100001 TEA 40#\n2 100002 SUGAR 25.5\n");
    TextRecordFile file(kPath);
    CHECK_THROWS(file.readAll());

    writeRaw("#\n");
    CHECK_THROWS(file.readAll());
}

static void testAppendAfterLineWithoutBreak() {
    writeRaw("1 a#");  // no line break at the end
    TextRecordFile file(kPath);
    file.append(record("2", "b"));
    List<List<std::string>> all = file.readAll();
    CHECK(all.size() == 2);
    CHECK(all[1][0] == "2");
}

static void testBadTokensAreRejected() {
    TextRecordFile file(kPath);
    file.removeFromDisk();
    CHECK_THROWS(file.append(List<std::string>()));
    CHECK_THROWS(file.append(record("a#b")));
    List<std::string> withEmpty;
    withEmpty.push_back("a");
    withEmpty.push_back("");
    CHECK_THROWS(file.append(withEmpty));
    CHECK(file.readAll().empty());  // nothing half-written
}

static void testRewrite() {
    TextRecordFile file(kPath);
    file.removeFromDisk();
    file.append(record("old1"));
    file.append(record("old2"));

    List<List<std::string>> fresh;
    fresh.push_back(record("new1", "x"));
    file.rewrite(fresh);

    CHECK(readRaw(kPath) == "new1 x#\n");
    std::ifstream temp((std::string(kPath) + ".tmp").c_str());
    CHECK(!temp.is_open());  // the temporary file was replaced, not left behind

    file.rewrite(List<List<std::string>>());  // rewriting with nothing empties the file
    CHECK(file.readAll().empty());
}

static void testFolderThatDoesNotExist() {
    TextRecordFile file("no_such_folder_xyz/data.txt");
    CHECK_THROWS(file.readAll());
}

static void testStringHelpers() {
    CHECK(pos::trim("  a b \r\n") == "a b");
    CHECK(pos::split("a  b c").size() == 3);
    CHECK(pos::join(pos::split("a b c"), "-") == "a-b-c");
    CHECK(pos::padLeft("42", 6) == "000042");
    CHECK(pos::stripEndMarker("x y #") == "x y");
    CHECK(pos::stripEndMarker("x y#") == "x y");
    CHECK(pos::formatQuantity(2) == "2");
    CHECK(pos::formatQuantity(1.5) == "1.5");
    CHECK(pos::formatQuantity(25.5) == "25.5");
    CHECK(pos::formatQuantity(0.125) == "0.125");
    double d = 0;
    CHECK(pos::parseDouble("25.5", d) && d == 25.5);
    CHECK(!pos::parseDouble("1.", d));
    CHECK(!pos::parseDouble("abc", d));
    CHECK(!pos::parseDouble("1e5", d));
    long long n = 0;
    CHECK(pos::parseInt("000123", n) && n == 123);
    CHECK(!pos::parseInt("12a", n));
    CHECK(!pos::parseInt("", n));
}

int main() {
    testMissingFileIsCreated();
    testAppendAndReadBack();
    testReadingHandMadeFile();
    testMissingMarkerIsRejected();
    testAppendAfterLineWithoutBreak();
    testBadTokensAreRejected();
    testRewrite();
    testFolderThatDoesNotExist();
    testStringHelpers();

    TextRecordFile(kPath).removeFromDisk();
    if (failures == 0) {
        std::cout << "test_text_record_file: all checks passed" << std::endl;
        return 0;
    }
    std::cout << "test_text_record_file: " << failures << " check(s) failed" << std::endl;
    return 1;
}
