
#include <iostream>
#include <string>

#include "barcode/decoder.hpp"
#include "barcode/error.hpp"

using namespace std;

static bool check(const string& name, const string& path, const string& expected) {
    try {
        string result = decodeFromFile(path);
        if (result == expected) {
            cout << name << ": PASS" << endl;
            return true;
        }
        cout << name << ": FAIL (expected " << expected
             << ", got " << result << ")" << endl;
    } catch (const BarcodeError& e) {
        cout << name << ": FAIL (" << e.what() << ")" << endl;
    }
    return false;
}

int main() {
    bool ok = true;
    ok &= check("Clean image", "samples/clean.bmp", "123456");
    ok &= check("Noisy image", "samples/noisy.bmp", "654321");
    ok &= check("Upside-down image", "samples/upside_down.bmp", "987654");

    if (ok) {
        cout << "test_samples: all passed" << endl;
        return 0;
    }
    cout << "test_samples: some samples failed" << endl;
    return 1;
}
