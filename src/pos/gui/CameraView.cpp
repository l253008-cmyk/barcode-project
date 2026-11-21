// CameraView.cpp                                     Owner: Person 1

#include "pos/gui/CameraView.hpp"

#include <algorithm>
#include <cstddef>

#include <QPainter>
#include <QPen>

CameraView::CameraView(QWidget* parent)
    : QWidget(parent), message_(QStringLiteral("Waiting for the camera...")) {
    setMinimumSize(320, 240);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void CameraView::showFrame(const Image& frame) {
    const std::size_t expected = static_cast<std::size_t>(frame.width) * frame.height;
    if (frame.width <= 0 || frame.height <= 0 || frame.pixels.size() != expected) {
        return;
    }
    // View the vector's memory as a QImage, then copy it so the picture
    // stays valid after the frame is gone.
    const QImage view(frame.pixels.data(), frame.width, frame.height, frame.width,
        QImage::Format_Grayscale8);
    frame_ = view.copy();
    update();
}

void CameraView::showMessage(const QString& text) {
    frame_ = QImage();
    message_ = text;
    update();
}

QSize CameraView::sizeHint() const {
    return QSize(640, 480);
}

void CameraView::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    if (frame_.isNull()) {
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter | Qt::TextWordWrap, message_);
        return;
    }

    // Draw the picture as large as possible without changing its proportions.
    const double scale = std::min(static_cast<double>(width()) / frame_.width(),
        static_cast<double>(height()) / frame_.height());
    const double drawWidth = frame_.width() * scale;
    const double drawHeight = frame_.height() * scale;
    const QRectF target((width() - drawWidth) / 2.0, (height() - drawHeight) / 2.0,
        drawWidth, drawHeight);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawImage(target, frame_);

    // Guide rectangle in the middle: the decoder reads rows around the centre.
    const QRectF guide(target.x() + target.width() * 0.15,
        target.y() + target.height() * 0.30,
        target.width() * 0.70, target.height() * 0.40);
    QPen pen(QColor(0, 200, 0));
    pen.setWidth(2);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(guide);

    // The hint goes on a dark strip at the bottom, so it is readable over any picture.
    const QRectF strip(0, height() - 28, width(), 28);
    painter.fillRect(strip, QColor(0, 0, 0, 170));
    painter.setPen(Qt::white);
    painter.drawText(strip, Qt::AlignCenter, QStringLiteral("Hold the barcode inside the box"));
}