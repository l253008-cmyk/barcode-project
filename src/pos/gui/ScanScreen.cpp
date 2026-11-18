#include "pos/gui/ScanScreen.hpp"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "barcode/error.hpp"
#include "pos/common/PosError.hpp"
#include "pos/gui/CameraView.hpp"
#include "pos/gui/ProductDetailsWidget.hpp"
#include "pos/scanning/FileFrameSource.hpp"
#include "pos/scanning/IBarcodeReader.hpp"
#include "pos/services/ProductService.hpp"

ScanScreen::ScanScreen(ProductService& products, IBarcodeReader& reader, QWidget* parent)
    : QWidget(parent), products_(products), reader_(reader) {
    camera_ = new CameraView(this);
    details_ = new ProductDetailsWidget(this);
    status_ = new QLabel("Hold a barcode in front of the camera.", this);

    QPushButton* openButton = new QPushButton("Open Image", this);
    connect(openButton, &QPushButton::clicked, this, &ScanScreen::openImage);

    QVBoxLayout* left = new QVBoxLayout();
    left->addWidget(camera_, 1);
    left->addWidget(openButton);
    left->addWidget(status_);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addLayout(left, 2);
    layout->addWidget(details_, 1);
}

void ScanScreen::showFrame(const Image& frame) {
    camera_->showFrame(frame);
}

void ScanScreen::handleCode(const std::string& code) {
    status_->setText("Scanned: " + QString::fromStdString(code));
    try {
        details_->showDescription(QString::fromStdString(products_.describeByBarcode(code)));
    } catch (const PosError& error) {
        details_->showMessage(QString::fromUtf8(error.what()));
    } catch (const BarcodeError& error) {
        details_->showMessage(QString::fromUtf8(error.what()));
    }
}

void ScanScreen::openImage() {
    QString path = QFileDialog::getOpenFileName(this, "Open barcode image", "", "Images (*.bmp)");
    if (path.isEmpty()) {
        return;
    }

    try {
        Image image;
        bool loaded = false;
        FileFrameSource source(path.toStdString());
        source.setFrameCallback([&image, &loaded](const Image& frame) {
            image = frame;
            loaded = true;
        });
        source.start();
        source.stop();

        if (!loaded) {
            status_->setText("Could not load the image.");
            return;
        }

        auto code = reader_.read(image);
        if (!code) {
            status_->setText("No barcode was found in the image.");
            return;
        }
        handleCode(*code);
    } catch (const PosError& error) {
        status_->setText(QString::fromUtf8(error.what()));
    } catch (const BarcodeError& error) {
        status_->setText(QString::fromUtf8(error.what()));
    }
}
