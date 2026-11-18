// FileFrameSource.cpp                                Owner: Person 1

#include "pos/scanning/FileFrameSource.hpp"

#include <utility>

#include "barcode/error.hpp"
#include "barcode/image_io.hpp"

FileFrameSource::FileFrameSource(std::string path, int repeat)
    : path_(std::move(path)), repeat_(repeat < 1 ? 1 : repeat) {
}

void FileFrameSource::setFrameCallback(FrameCallback callback) {
    onFrame_ = std::move(callback);
}

void FileFrameSource::setErrorCallback(ErrorCallback callback) {
    onError_ = std::move(callback);
}

void FileFrameSource::start() {
    // Only the loading is guarded. An exception thrown by the frame callback
    // is not a file error and must not be reported as one.
    Image frame;
    try {
        frame = loadImage(path_);
    }
    catch (const BarcodeError& error) {
        if (onError_) {
            onError_(error.what());
        }
        return;
    }

    running_ = true;
    for (int i = 0; i < repeat_ && running_; i++) {
        if (onFrame_) {
            onFrame_(frame);
        }
    }
    running_ = false;
}

void FileFrameSource::stop() {
    running_ = false;
}