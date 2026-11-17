#pragma once

#include <QWidget>
#include <string>

#include "pos/scanning/IFrameSource.hpp"

class CameraView;
class ProductDetailsWidget;
class ProductService;
class IBarcodeReader;
class QLabel;

class ScanScreen : public QWidget {
    Q_OBJECT

public:
    ScanScreen(ProductService& products, IBarcodeReader& reader, QWidget* parent = nullptr);

    void showFrame(const Image& frame);
    void handleCode(const std::string& code);

private slots:
    void openImage();

private:
    ProductService& products_;
    IBarcodeReader& reader_;
    CameraView* camera_;
    ProductDetailsWidget* details_;
    QLabel* status_;
};
