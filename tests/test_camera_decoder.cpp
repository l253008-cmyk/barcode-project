#include "barcode/camera_decoder.hpp"
#include "barcode/image_io.hpp"
#include "barcode/error.hpp"
#include "test_utils.hpp"
#include <iostream>
#include <string>

#ifndef SAMPLES_DIR
#define SAMPLES_DIR "samples"
#endif

static bool checkSample(const std::string& name, const std::string& path, const std::string& expected) {
    try {
        Image img = loadImage(path);
        std::string code = decodeFrame(img);
        return checkResult(name, expected, code);
    }
    catch (const BarcodeError& e) {
        std::cout << name << " failed with error: " << e.what() << std::endl;
        return false;
    }
}

int main() {
    std::string dir = std::string(SAMPLES_DIR) + "/camera";
    bool ok = true;

    ok &= checkSample("100001 noise", dir + "/100001_noise.bmp", "100001");
    ok &= checkSample("100001 shadow", dir + "/100001_shadow.bmp", "100001");
    ok &= checkSample("100001 tilt", dir + "/100001_tilt.bmp", "100001");
    ok &= checkSample("100002 noise", dir + "/100002_noise.bmp", "100002");
    ok &= checkSample("100002 shadow", dir + "/100002_shadow.bmp", "100002");
    ok &= checkSample("100002 tilt", dir + "/100002_tilt.bmp", "100002");
    ok &= checkThrowsAs<BarcodeError>("100001 blur throws BarcodeError", [&]() {
        Image img = loadImage(dir + "/100001_blur.bmp");
        decodeFrame(img);
        });

    if (ok) {
        std::cout << "test_camera_decoder: all passed\n";
        return 0;
    }
    std::cout << "test_camera_decoder: some tests failed\n";
    return 1;
}