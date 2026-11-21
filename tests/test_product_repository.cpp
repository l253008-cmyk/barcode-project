
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include "pos/common/PosError.hpp"
#include "pos/storage/FileProductRepository.hpp"
using namespace std;

// the tests use a real file, it is deleted at the end
const string kTestFile = "test_products_data.txt";

// writes the lines into the test file, old content is replaced
static void writeFile(const vector<string>& lines) {
    ofstream out(kTestFile, ios::trunc);
    int count = static_cast<int>(lines.size());
    for (int i = 0; i < count; i++) {
        out << lines[i] << "\n";
    }
}

// reads the test file line by line
static vector<string> readFile() {
    ifstream in(kTestFile);
    vector<string> lines;
    string line;
    while (getline(in, line)) {
        lines.insert(lines.end(), 1, line);
    }
    return lines;
}

// the three products from the proposal
static vector<string> goldenLines() {
    vector<string> lines;
    lines.insert(lines.end(), 1, "1 100001 TEA Tetley Tea Bags 25 count 350.00 17.50 10.00 40 data/barcodes/100001.bmp#");
    lines.insert(lines.end(), 1, "2 100002 SUGAR Sugar Loose weight 180.00 0.00 0.00 25.5 data/barcodes/100002.bmp#");
    lines.insert(lines.end(), 1, "3 100003 BISCUIT Sooper Biscuit count 60.00 3.00 5.00 100 data/barcodes/100003.bmp#");
    return lines;
}

static bool allThrows(const FileProductRepository& repository) {
    try {
        repository.all();
    }
    catch (const PosError&) {
        return true;
    }
    return false;
}

static bool updateThrows(FileProductRepository& repository, int id, double quantity) {
    try {
        repository.updateQuantity(id, quantity);
    }
    catch (const PosError&) {
        return true;
    }
    return false;
}

int main() {
    // ---------- reading ----------
    writeFile(goldenLines());
    FileProductRepository repository(kTestFile);

    vector<Product> products = repository.all();
    assert(products.size() == 3);
    assert(products[0].acronym == "TEA");
    assert(products[1].acronym == "SUGAR");
    assert(products[2].acronym == "BISCUIT");

    assert(repository.findByBarcode("100002").value().name == "Sugar Loose");
    assert(repository.findByBarcode("100002").value().unit == MeasurementUnit::Weight);
    assert(repository.findByBarcode("100002").value().availableQty == 25.5);
    assert(repository.findById(3).value().acronym == "BISCUIT");
    assert(!repository.findByBarcode("999999").has_value());      // not in the file
    assert(!repository.findById(99).has_value());

    // ---------- updating the stock ----------
    repository.updateQuantity(1, 38);

    FileProductRepository reopened(kTestFile);                    // read the file again
    assert(reopened.all().size() == 3);
    assert(reopened.findById(1).value().availableQty == 38);
    assert(reopened.findById(1).value().name == "Tetley Tea Bags 25");
    assert(reopened.findById(2).value().availableQty == 25.5);    // others did not change
    assert(reopened.findById(3).value().availableQty == 100);

    vector<string> lines = readFile();
    assert(lines.size() == 3);
    assert(lines[0] == "1 100001 TEA Tetley Tea Bags 25 count 350.00 17.50 10.00 38 data/barcodes/100001.bmp#");

    ifstream leftover(kTestFile + ".tmp");                        // temporary file must be gone
    assert(!leftover.is_open());

    repository.updateQuantity(2, 20.25);
    assert(FileProductRepository(kTestFile).findById(2).value().availableQty == 20.25);
    repository.updateQuantity(3, 0);                              // stock can reach zero
    assert(FileProductRepository(kTestFile).findById(3).value().availableQty == 0);

    // ---------- wrong updates must not change the file ----------
    vector<string> before = readFile();
    assert(updateThrows(repository, 99, 5));                      // no such product
    assert(updateThrows(repository, 1, -1));                      // negative stock
    assert(readFile() == before);

    // ---------- wrong files ----------
    vector<string> badAcronym = goldenLines();
    badAcronym[1] = "2 100002 SUGARSUGAR Sugar Loose weight 180.00 0.00 0.00 25.5 a.bmp#";
    writeFile(badAcronym);
    FileProductRepository badRepository(kTestFile);
    assert(allThrows(badRepository));
    bool findThrows = false;
    try {
        badRepository.findByBarcode("100001");
    }
    catch (const PosError&) {
        findThrows = true;
    }
    assert(findThrows);

    vector<string> sameBarcode = goldenLines();
    sameBarcode[1] = "2 100001 SUGAR Sugar Loose weight 180.00 0.00 0.00 25.5 a.bmp#";
    writeFile(sameBarcode);
    assert(allThrows(FileProductRepository(kTestFile)));

    vector<string> sameId = goldenLines();
    sameId[1] = "1 100002 SUGAR Sugar Loose weight 180.00 0.00 0.00 25.5 a.bmp#";
    writeFile(sameId);
    assert(allThrows(FileProductRepository(kTestFile)));

    // ---------- a file that does not exist yet ----------
    remove(kTestFile.c_str());
    assert(FileProductRepository(kTestFile).all().empty());

    remove(kTestFile.c_str());
    remove((kTestFile + ".tmp").c_str());

    cout << "test_product_repository: all passed" << endl;
    return 0;
}
