// QtCameraSource.hpp                                 Owner: Person 1
// The real camera, using Qt Multimedia. It turns every camera frame into a
// grayscale Image and hands it to the frame callback.
// This is one of only two places (with the gui/ files) that use Qt.
//
// There is no Q_OBJECT here on purpose: the class connects to Qt signals with
// lambdas, so it needs no moc step and no change to the CMake file.

#ifndef POS_QT_CAMERA_SOURCE_HPP
#define POS_QT_CAMERA_SOURCE_HPP

#include <memory>

#include <QMediaCaptureSession>
#include <QObject>
#include <QVideoSink>

#include "pos/scanning/IFrameSource.hpp"

class QCamera;
class QVideoFrame;

class QtCameraSource : public IFrameSource {
public:
    QtCameraSource();
    ~QtCameraSource() override;

    QtCameraSource(const QtCameraSource&) = delete;
    QtCameraSource& operator=(const QtCameraSource&) = delete;

    void setFrameCallback(FrameCallback callback) override;
    void setErrorCallback(ErrorCallback callback) override;

    // Opens the default camera (frame size close to AppConfig's preferred size).
    // If there is no camera, the error callback gets a message.
    void start() override;
    void stop() override;

private:
    void handleFrame(const QVideoFrame& frame);
    void reportError(const std::string& message);

    // The order matters: members are destroyed in reverse order,
    // so the camera goes first and the context object goes last.
    QObject context_;
    QVideoSink sink_;
    QMediaCaptureSession session_;
    std::unique_ptr<QCamera> camera_;

    FrameCallback onFrame_;
    ErrorCallback onError_;
};

#endif  // POS_QT_CAMERA_SOURCE_HPP