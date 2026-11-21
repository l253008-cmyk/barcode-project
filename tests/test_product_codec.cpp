
#include <cassert>
#include <iostream>
#include "pos/common/PosError.hpp"
#include "pos/storage/ProductCodec.hpp"
using namespace std;

// split a line on spaces
static vector<string> tokensOf(const string& line) {
    vector<string> tokens;
    string word;
    int length = static_cast<int>(line.size());
    for (int i = 0; i < length; i++) {
        if (line[i] == ' ') {
            tokens.insert(tokens.end(), 1, word);
            word = "";
        }
        else {
            word += line[i];
        }
    }
    tokens.insert(tokens.end(), 1, word);
    return tokens;
}

// join tokens back with one space between them
static string joinTokens(const vector<string>& tokens) {
    string line;
    int count = static_cast<int>(tokens.size());
    for (int i = 0; i < count; i++) {
        if (i > 0) {
            line += " ";
        }
        line += tokens[i];
    }
    return line;
}

// true when parsing the line throws a PosError
static bool parseThrows(const string& line) {
    try {
        ProductCodec::parse(tokensOf(line));
    }
    catch (const PosError&) {
        return true;
    }
    return false;
}

int main() {
    // the three lines from the proposal, without the # at the end
    const string teaLine = "1 100001 TEA Tetley Tea Bags 25 count 350.00 17.50 10.00 40 data/barcodes/100001.bmp";
    const string sugarLine = "2 100002 SUGAR Sugar Loose weight 180.00 0.00 0.00 25.5 data/barcodes/100002.bmp";
    const string biscuitLine = "3 100003 BISCUIT Sooper Biscuit count 60.00 3.00 5.00 100 data/barcodes/100003.bmp";

    // ---------- parse the good lines ----------
    Product tea = ProductCodec::parse(tokensOf(teaLine));
    assert(tea.id == 1);
    assert(tea.barcode == "100001");
    assert(tea.acronym == "TEA");
    assert(tea.name == "Tetley Tea Bags 25");          // name has 4 words
    assert(tea.unit == MeasurementUnit::Count);
    assert(tea.unitPrice.toString() == "350.00");
    assert(tea.tax.toString() == "17.50");
    assert(tea.discount.toString() == "10.00");
    assert(tea.availableQty == 40);
    assert(tea.barcodePath == "data/barcodes/100001.bmp");

    Product sugar = ProductCodec::parse(tokensOf(sugarLine));
    assert(sugar.name == "Sugar Loose");
    assert(sugar.unit == MeasurementUnit::Weight);
    assert(sugar.availableQty == 25.5);
    assert(sugar.discount.toString() == "0.00");

    Product biscuit = ProductCodec::parse(tokensOf(biscuitLine));
    assert(biscuit.acronym == "BISCUIT");
    assert(biscuit.name == "Sooper Biscuit");
    assert(biscuit.availableQty == 100);

    // ---------- one word name, acronym with exactly 8 letters ----------
    Product oneWord = ProductCodec::parse(tokensOf("4 100004 TEABAGS1 Tea count 10.00 0.00 0.00 5 a.bmp"));
    assert(oneWord.name == "Tea");
    assert(oneWord.acronym == "TEABAGS1");

    // ---------- format gives the same line back ----------
    assert(joinTokens(ProductCodec::format(tea)) == teaLine);
    assert(joinTokens(ProductCodec::format(sugar)) == sugarLine);
    assert(joinTokens(ProductCodec::format(biscuit)) == biscuitLine);

    // ---------- parse after format gives the same product ----------
    Product again = ProductCodec::parse(ProductCodec::format(tea));
    assert(again.id == tea.id);
    assert(again.name == tea.name);
    assert(again.unitPrice == tea.unitPrice);
    assert(again.availableQty == tea.availableQty);

    // ---------- bad lines must throw ----------
    assert(parseThrows("1 100001 TEABAGS12 Tetley count 350.00 17.50 10.00 40 a.bmp"));   // acronym has 9 letters
    assert(parseThrows("1 12345 TEA Tetley count 350.00 17.50 10.00 40 a.bmp"));          // barcode has 5 digits
    assert(parseThrows("1 012345 TEA Tetley count 350.00 17.50 10.00 40 a.bmp"));         // barcode starts with 0
    assert(parseThrows("1 12a456 TEA Tetley count 350.00 17.50 10.00 40 a.bmp"));         // barcode has a letter
    assert(parseThrows("1 100001 TEA Tetley count abc 17.50 10.00 40 a.bmp"));            // price is not a number
    assert(parseThrows("1 100001 TEA Tetley count -5.00 17.50 10.00 40 a.bmp"));          // price is negative
    assert(parseThrows("1 100001 TEA Tetley count 350.00 abc 10.00 40 a.bmp"));           // tax is not a number
    assert(parseThrows("1 100001 TEA Tetley count 350.00 17.50 abc 40 a.bmp"));           // discount is not a number
    assert(parseThrows("1 100001 TEA Tetley count 350.00 17.50 10.00 abc a.bmp"));        // quantity is not a number
    assert(parseThrows("1 100001 TEA Tetley count 350.00 17.50 10.00 1.2.3 a.bmp"));      // two dots
    assert(parseThrows("1 100001 TEA Tetley count 350.00 17.50 10.00 -4 a.bmp"));         // negative quantity
    assert(parseThrows("1 100001 TEA Tetley count 350.00 17.50 10.00 40.5 a.bmp"));       // half a packet
    assert(parseThrows("1 100001 TEA Tetley pieces 350.00 17.50 10.00 40 a.bmp"));        // unknown unit
    assert(parseThrows("x 100001 TEA Tetley count 350.00 17.50 10.00 40 a.bmp"));         // id is not a number
    assert(parseThrows("0 100001 TEA Tetley count 350.00 17.50 10.00 40 a.bmp"));         // id is zero
    assert(parseThrows("1 100001 TEA count 350.00 17.50 10.00 40 a.bmp"));                // no name, only 9 fields
    assert(parseThrows("1 100001 TEA"));                                                  // too short
    assert(parseThrows(""));                                                              // empty line

    cout << "test_product_codec: all passed" << endl;
    return 0;
}
