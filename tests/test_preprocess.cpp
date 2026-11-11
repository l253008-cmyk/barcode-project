#include "barcode/preprocess.hpp"
#include "test_utils.hpp"

int main() {
    bool ok = true;

    Image gray;
    gray.width = 2;
    gray.height = 2;
    gray.pixels = { 10, 30, 150, 210 };

    Image binary = toBinary(gray);

    // Average is 100, so 10 and 30 become black, 150 and 210 become white.
    ok &= checkTrue("Preprocessing test",
        binary.width == 2 && binary.height == 2 &&
        binary.at(0, 0) == 0 && binary.at(1, 0) == 0 &&
        binary.at(0, 1) == 255 && binary.at(1, 1) == 255);

    ok &= checkTrue("Original image unchanged test",
        gray.at(0, 0) == 10 && gray.at(1, 0) == 30 &&
        gray.at(0, 1) == 150 && gray.at(1, 1) == 210);

    Image empty;
    ok &= checkThrows("Empty image test", [&]() { toBinary(empty); });

    return ok ? 0 : 1;
}
