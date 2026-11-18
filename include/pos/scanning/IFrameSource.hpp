// IFrameSource.hpp                                   Owner: Person 1
// Anything that can deliver camera-like frames: a real camera, or an image file.
// The scanning code only knows this interface, so a new source (for example a
// phone camera) can be added without changing any existing code.

#ifndef POS_IFRAME_SOURCE_HPP
#define POS_IFRAME_SOURCE_HPP

#include <functional>
#include <string>

#include "barcode/image.hpp"

class IFrameSource {
public:
    using FrameCallback = std::function<void(const Image&)>;
    using ErrorCallback = std::function<void(const std::string&)>;

    virtual ~IFrameSource() = default;

    // Called for every frame the source delivers (grayscale, 0 = black, 255 = white).
    virtual void setFrameCallback(FrameCallback callback) = 0;

    // Called with a readable message when the source cannot work
    // (no camera, permission denied, unreadable file).
    virtual void setErrorCallback(ErrorCallback callback) = 0;

    virtual void start() = 0;
    virtual void stop() = 0;
};

#endif  // POS_IFRAME_SOURCE_HPP