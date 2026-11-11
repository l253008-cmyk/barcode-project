#include "barcode/encoder.hpp"
#include "barcode/decoder.hpp"
#include "test_utils.hpp"
#include <iostream>
#include <string>
using namespace std;
static bool testMemoryRoundTrip(const string& code) {
    string result;
    try {
        result = decodeBarcode(generateBarcode(code));
    }
    catch (const BarcodeError&) {
        result = "(exception)";
    }
    return checkResult("Memory round trip for " + code, code, result);
}
static bool testFileRoundTrip(const string& code) {
    const string path = "roundtrip_test.bmp";
    string result;
    try {
        generateToFile(code, path);
        result = decodeFromFile(path);
    }
    catch (const BarcodeError&) {
        result = "(exception)";
    }
    return checkResult("File round trip for " + code, code, result);
}
static bool testRejected(const string& code) {
    return checkThrows("Rejected code " + code, [&]() { generateBarcode(code); });
}
int main() {
    bool ok = true;
    const string goodCodes[] = { "123456", "654321", "987654", "999999", "900001" };
    for (const string& code : goodCodes) {
        ok &= testMemoryRoundTrip(code);
    }
    ok &= testFileRoundTrip("123456");
    ok &= testFileRoundTrip("900001");
    const string badCodes[] = { "012345", "000000", "12345", "1234567", "12a456" };
    for (const string& code : badCodes) {
        ok &= testRejected(code);
    }
    if (ok) {
        cout << "All round trip tests passed!" << endl;
        return 0;
    }
    cout << "Some round trip tests failed!" << endl;
    return 1;
}