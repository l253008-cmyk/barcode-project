// ScanController.hpp                                  Owner: Person 1
// Decides when a camera frame scan counts as a valid read.
// Manages frame rate throttling (skip < 250ms), consecutive confirmations (2 reads),
// and repeat code debounce (ignore same code for 2s). Qt-free.

#ifndef POS_SCAN_CONTROLLER_HPP
#define POS_SCAN_CONTROLLER_HPP

#include <functional>
#include <string>

#include "barcode/image.hpp"
#include "pos/common/IClock.hpp"
#include "pos/scanning/IBarcodeReader.hpp"

class ScanController {
public:
    using CodeCallback = std::function<void(const std::string&)>;

    ScanController(IBarcodeReader& reader, const IClock& clock);

    // Register callback triggered when a barcode is confirmed
    void setOnCode(CodeCallback callback);

    // Process an incoming video/camera frame
    void onFrame(const Image& frame);

    // Reset internal state
    void reset();

private:
    IBarcodeReader& reader_;
    const IClock& clock_;
    CodeCallback onCode_;

    long long lastAttemptTime_ = -1000000;
    long long lastReportedTime_ = -1000000;
    std::string lastReportedCode_;

    std::string candidateCode_;
    int consecutiveCount_ = 0;
    long long lastCandidateTime_ = -1000000;
};

#endif  // POS_SCAN_CONTROLLER_HPP
