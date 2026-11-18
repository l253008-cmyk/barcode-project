// ScanController.cpp                                  Owner: Person 1
// Implements frame throttling, repeat suppression, and reading confirmation.

#include "pos/scanning/ScanController.hpp"

#include "pos/common/AppConfig.hpp"

ScanController::ScanController(IBarcodeReader& reader, const IClock& clock)
    : reader_(reader), clock_(clock) {}

void ScanController::setOnCode(CodeCallback callback) {
    onCode_ = callback;
}

void ScanController::reset() {
    lastAttemptTime_ = -1000000;
    lastReportedTime_ = -1000000;
    lastReportedCode_.clear();
    candidateCode_.clear();
    consecutiveCount_ = 0;
    lastCandidateTime_ = -1000000;
}

void ScanController::onFrame(const Image& frame) {
    long long now = clock_.millis();

    // Skip frames that arrive within 250 ms of the last attempt
    if (lastAttemptTime_ >= 0 && (now - lastAttemptTime_) < pos::config::kScanIntervalMs) {
        return;
    }
    lastAttemptTime_ = now;

    auto maybeCode = reader_.read(frame);
    if (!maybeCode.has_value() || maybeCode->empty()) {
        // Frame does not contain a valid barcode.
        // If more than 800ms has elapsed since the candidate was detected, reset it.
        // This provides a grace window against intermediate motion blur or camera sensor flicker.
        if (lastCandidateTime_ >= 0 && (now - lastCandidateTime_) > 800) {
            candidateCode_.clear();
            consecutiveCount_ = 0;
        }
        return;
    }

    const std::string& code = *maybeCode;

    // Ignore the same code for 2 seconds after reporting
    if (code == lastReportedCode_ && (now - lastReportedTime_) < pos::config::kRepeatIgnoreMs) {
        candidateCode_.clear();
        consecutiveCount_ = 0;
        return;
    }

    // Check consecutive agreeing frames
    if (code == candidateCode_) {
        ++consecutiveCount_;
        lastCandidateTime_ = now;
    } else {
        candidateCode_ = code;
        consecutiveCount_ = 1;
        lastCandidateTime_ = now;
    }

    // Require 2 consecutive equal reads
    if (consecutiveCount_ >= pos::config::kConfirmReads) {
        lastReportedCode_ = code;
        lastReportedTime_ = now;
        candidateCode_.clear();
        consecutiveCount_ = 0;

        if (onCode_) {
            onCode_(code);
        }
    }
}
