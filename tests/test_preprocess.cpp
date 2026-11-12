#include "barcode/preprocess.hpp"
#include "test_utils.hpp"

// True if pixel x of a 1-row image is black exactly when it should be
// (two bars: x 20 to 24 and x 70 to 74).
static bool isBarPixel(int x) {
    return (x >= 20 && x <= 24) || (x >= 70 && x <= 74);
}

int main() {
    bool ok = true;

    // 1. Clean image: two black bars on white stay exactly as they are.
    Image clean;
    clean.width = 100;
    clean.height = 1;
    clean.pixels.resize(100, 255);
    for (int x = 0; x < 100; x++) {
        if (isBarPixel(x)) clean.at(x, 0) = 0;
    }
    Image cleanBinary = toBinary(clean);
    bool cleanOk = true;
    for (int x = 0; x < 100; x++) {
        if ((cleanBinary.at(x, 0) == 0) != isBarPixel(x)) cleanOk = false;
    }
    ok &= checkTrue("Clean image test", cleanOk);

    // 2. Shadow: light goes from 40% (left) to 100% (right).
    //    A single average threshold fails here, the adaptive one must not.
    Image shadow;
    shadow.width = 100;
    shadow.height = 1;
    shadow.pixels.resize(100);
    for (int x = 0; x < 100; x++) {
        double light = 0.4 + 0.6 * x / 100.0;
        double base = isBarPixel(x) ? 20.0 : 255.0;
        shadow.at(x, 0) = static_cast<unsigned char>(base * light);
    }
    Image shadowBinary = toBinary(shadow);
    bool shadowOk = true;
    for (int x = 0; x < 100; x++) {
        if ((shadowBinary.at(x, 0) == 0) != isBarPixel(x)) shadowOk = false;
    }
    ok &= checkTrue("Shadow test", shadowOk);

    // 3. Output has only 0 and 255, and the original image is not changed.
    bool onlyTwoValues = true;
    for (unsigned char p : shadowBinary.pixels) {
        if (p != 0 && p != 255) onlyTwoValues = false;
    }
    ok &= checkTrue("Only black and white test", onlyTwoValues);
    ok &= checkTrue("Original image unchanged test",
        shadow.at(0, 0) == static_cast<unsigned char>(255 * 0.4) &&
        shadow.at(99, 0) == static_cast<unsigned char>(255 * 0.994));

    // 4. A plain white image stays white (no barcode, nothing turns black).
    Image white;
    white.width = 40;
    white.height = 3;
    white.pixels.resize(120, 255);
    Image whiteBinary = toBinary(white);
    bool allWhite = true;
    for (unsigned char p : whiteBinary.pixels) {
        if (p != 255) allWhite = false;
    }
    ok &= checkTrue("Plain white image test", allWhite);

    // 5. An empty image must throw.
    Image empty;
    ok &= checkThrows("Empty image test", [&]() { toBinary(empty); });

    return ok ? 0 : 1;
}