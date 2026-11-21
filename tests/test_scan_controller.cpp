// test_scan_controller.cpp                            Owner: Person 1
// Tests ScanController using a fake reader and fake clock.

#include <iostream>
#include <string>
#include <vector>

#include "barcode/image.hpp"
#include "pos/common/IClock.hpp"
#include "pos/scanning/IBarcodeReader.hpp"
#include "pos/scanning/ScanController.hpp"
#include "test_utils.hpp"

namespace {

class FakeClock : public IClock {
public:
    long long currentMillis = 0;
    std::string today() const override { return "2026-11-18"; }
    std::string now() const override { return "12:00:00"; }
    long long millis() const override { return currentMillis; }
    void advance(long long ms) { currentMillis += ms; }
};

class FakeBarcodeReader : public IBarcodeReader {
public:
    std::optional<std::string> nextCode;
    int readCallCount = 0;

    std::optional<std::string> read(const Image&) override {
        ++readCallCount;
        return nextCode;
    }
};

}  // namespace

int main() {
    bool ok = true;
    FakeClock clock;
    FakeBarcodeReader reader;
    ScanController controller(reader, clock);

    std::vector<std::string> reportedCodes;
    controller.setOnCode([&reportedCodes](const std::string& code) {
        reportedCodes.push_back(code);
    });

    Image dummyFrame;
    dummyFrame.width = 10;
    dummyFrame.height = 10;
    dummyFrame.pixels.resize(100, 255);

    // 1. Two consecutive reads required
    reader.nextCode = "100001";
    clock.currentMillis = 0;
    controller.onFrame(dummyFrame);
    ok &= checkTrue("First read does not report yet", reportedCodes.empty());
    ok &= checkTrue("Reader called once", reader.readCallCount == 1);

    // 2. Throttling: frame arriving within 250ms must be skipped
    clock.advance(100);  // now at 100ms (< 250ms)
    controller.onFrame(dummyFrame);
    ok &= checkTrue("Frame at 100ms skipped", reader.readCallCount == 1);

    // 3. Second matching frame at >= 250ms confirms the code
    clock.advance(200);  // now at 300ms
    controller.onFrame(dummyFrame);
    ok &= checkTrue("Reader called on second valid interval", reader.readCallCount == 2);
    ok &= checkTrue("Reported after 2 consecutive reads", reportedCodes.size() == 1);
    if (!reportedCodes.empty()) {
        ok &= checkResult("Reported code matches", "100001", reportedCodes[0]);
    }

    // 4. Repeat suppression: ignore same code for 2000ms
    clock.advance(300);  // at 600ms (< 300ms + 2000ms)
    controller.onFrame(dummyFrame);
    clock.advance(300);  // at 900ms
    controller.onFrame(dummyFrame);
    ok &= checkTrue("Same code suppressed within 2 seconds", reportedCodes.size() == 1);

    // 5. After 2000ms, the same code can be read again (after 2 consecutive reads)
    clock.advance(1800);  // at 2700ms (> 300 + 2000)
    controller.onFrame(dummyFrame);  // first read
    ok &= checkTrue("Not reported on first read after debounce", reportedCodes.size() == 1);
    clock.advance(300);   // at 3000ms
    controller.onFrame(dummyFrame);  // second read
    ok &= checkTrue("Reported again after debounce elapsed", reportedCodes.size() == 2);

    // 6. Non-matching consecutive reads reset candidate
    reader.nextCode = "100002";
    clock.advance(300);
    controller.onFrame(dummyFrame);  // candidate is 100002, count 1

    reader.nextCode = "100003";
    clock.advance(300);
    controller.onFrame(dummyFrame);  // candidate switched to 100003, count 1
    ok &= checkTrue("No report on switching code", reportedCodes.size() == 2);

    clock.advance(300);
    controller.onFrame(dummyFrame);  // candidate 100003, count 2 -> reports!
    ok &= checkTrue("100003 confirmed and reported", reportedCodes.size() == 3);
    if (reportedCodes.size() >= 3) {
        ok &= checkResult("Reported code is 100003", "100003", reportedCodes[2]);
    }

    if (ok) {
        std::cout << "test_scan_controller: all passed\n";
        return 0;
    }
    std::cout << "test_scan_controller: some tests failed\n";
    return 1;
}
