// FileFrameSource.hpp                                Owner: Person 1
// A frame source that "plays" an image file (BMP or PGM) instead of a camera.
// Used in tests, and as a backup in the demo when the camera does not work.

#ifndef POS_FILE_FRAME_SOURCE_HPP
#define POS_FILE_FRAME_SOURCE_HPP

#include <string>

#include "pos/scanning/IFrameSource.hpp"

class FileFrameSource : public IFrameSource {
public:
    // repeat = how many times the same frame is delivered by start().
    // A real camera sends the same barcode many times in a row, so tests use repeat > 1.
    explicit FileFrameSource(std::string path, int repeat = 1);

    void setFrameCallback(FrameCallback callback) override;
    void setErrorCallback(ErrorCallback callback) override;

    // Loads the file and delivers the frame. If the file cannot be read,
    // the error callback gets the message and no frame is delivered.
    void start() override;

    // Stops delivering. Can be called from inside the frame callback.
    void stop() override;

private:
    std::string path_;
    int repeat_;
    bool running_ = false;
    FrameCallback onFrame_;
    ErrorCallback onError_;
};

#endif  // POS_FILE_FRAME_SOURCE_HPP