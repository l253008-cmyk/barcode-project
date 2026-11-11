

#include <iostream>
#include <string>

#include "barcode/decoder.hpp"
#include "barcode/error.hpp"

using namespace std;
#ifndef SAMPLES_DIR
#define SAMPLES_DIR "samples"
#endif

static bool check(const string& name, const string& path, const string& expected) {
    try {
        string result = decodeFromFile(path);
        if (result == expected) {
            cout << name << ": PASS" << endl;
            return true;
        }
        cout << name << ": FAIL (expected " << expected
            << ", got " << result << ")" << endl;
    }
    catch (const BarcodeError& e) {
        cout << name << ": FAIL (" << e.what() << ")" << endl;
    }
    return false;
}

int main() {
    const string dir = SAMPLES_DIR;

    bool ok = true;
    ok &= check("Clean image", dir + "/clean.bmp", "123456");
    ok &= check("Noisy image", dir + "/noisy.bmp", "654321");
    ok &= check("Upside-down image", dir + "/upside_down.bmp", "987654");

    if (ok) {
        cout << "test_samples: all passed" << endl;
        return 0;
    }
    cout << "test_samples: some samples failed" << endl;
    return 1;
}