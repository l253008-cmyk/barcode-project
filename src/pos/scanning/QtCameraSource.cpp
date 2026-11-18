// QtCameraSource.cpp                                 Owner: Person 1

#include "pos/scanning/QtCameraSource.hpp"

#include <cstdlib>
#include <cstring>

#include <QCamera>
#include <QCameraDevice>
#include <QCameraFormat>
#include <QImage>
#include <QMediaDevices>
#include <QVideoFrame>

#include "pos/common/AppConfig.hpp"

namespace {

    // Picks the camera format whose size is closest to the preferred size.
    QCameraFormat closestFormat(const QCameraDevice& device) {
        QCameraFormat best;
        long long bestDistance = -1;
        for (const QCameraFormat& format : device.videoFormats()) {
            const QSize size = format.resolution();
            const long long distance =
                std::llabs(static_cast<long long>(size.width()) - config::kCameraWidth) +
                std::llabs(static_cast<long long>(size.height()) - config::kCameraHeight);
            if (bestDistance < 0 || distance < bestDistance) {
                bestDistance = distance;
                best = format;
            }
        }
        return best;
    }

    // Qt frame -> our Image (grayscale, one byte per pixel).
    // Returns an Image with width 0 if the frame has no picture.
    Image toImage(const QVideoFrame& frame) {
        Image result;
        QImage picture = frame.toImage();
        if (picture.isNull()) {
            return result;
        }
        picture = picture.convertToFormat(QImage::Format_Grayscale8);

        const int width = picture.width();
        const int height = picture.height();
        result.width = width;
        result.height = height;
        result.pixels.resize(static_cast<std::size_t>(width) * height);
        for (int y = 0; y < height; y++) {
            std::memcpy(result.pixels.data() + static_cast<std::size_t>(y) * width,
                picture.constScanLine(y), static_cast<std::size_t>(width));
        }
        return result;
    }

}  // namespace

QtCameraSource::QtCameraSource() {
    session_.setVideoSink(&sink_);
    // The lambda runs on the thread that owns context_ (the GUI thread),
    // so the frame callback can safely update widgets.
    QObject::connect(&sink_, &QVideoSink::videoFrameChanged, &context_,
        [this](const QVideoFrame& frame) { handleFrame(frame); });
}

QtCameraSource::~QtCameraSource() {
    stop();
    session_.setCamera(nullptr);
}

void QtCameraSource::setFrameCallback(FrameCallback callback) {
    onFrame_ = std::move(callback);
}

void QtCameraSource::setErrorCallback(ErrorCallback callback) {
    onError_ = std::move(callback);
}

void QtCameraSource::start() {
    if (!camera_) {
        const QCameraDevice device = QMediaDevices::defaultVideoInput();
        if (device.isNull()) {
            reportError("No camera was found. Connect a camera and check that "
                "Windows allows desktop apps to use it.");
            return;
        }

        camera_ = std::make_unique<QCamera>(device);
        const QCameraFormat format = closestFormat(device);
        if (!format.isNull()) {
            camera_->setCameraFormat(format);
        }
        QObject::connect(camera_.get(), &QCamera::errorOccurred, &context_,
            [this](QCamera::Error, const QString& text) {
                reportError(text.toStdString());
            });
        session_.setCamera(camera_.get());
    }
    camera_->start();
}

void QtCameraSource::stop() {
    if (camera_) {
        camera_->stop();
    }
}

void QtCameraSource::handleFrame(const QVideoFrame& frame) {
    if (!onFrame_) {
        return;
    }
    const Image image = toImage(frame);
    if (image.width > 0 && image.height > 0) {
        onFrame_(image);
    }
}

void QtCameraSource::reportError(const std::string& message) {
    if (onError_) {
        onError_(message);
    }
}