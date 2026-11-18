// CameraView.hpp                                     Owner: Person 1
// A widget that shows the live camera picture with a guide rectangle.
// Used by both the order screen and the scan screen.
// It only draws what it is given: connect it to any IFrameSource by calling
// showFrame from the frame callback. It knows nothing about cameras or barcodes.

#ifndef POS_CAMERA_VIEW_HPP
#define POS_CAMERA_VIEW_HPP

#include <QImage>
#include <QString>
#include <QWidget>

#include "barcode/image.hpp"

class CameraView : public QWidget {
public:
    explicit CameraView(QWidget* parent = nullptr);

    // Shows one frame (grayscale Image). Frames with a wrong size are ignored.
    void showFrame(const Image& frame);

    // Replaces the picture with a text, for example 'No camera was found'.
    void showMessage(const QString& text);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QImage frame_;
    QString message_;
};

#endif  // POS_CAMERA_VIEW_HPP