#pragma once

#include <QMainWindow>
#include <string>

#include "pos/gui/RoleDialog.hpp"
#include "pos/scanning/IFrameSource.hpp"

class ScanController;
class IBarcodeReader;
class ProductService;
class OrderService;
class CheckoutService;
class OrderScreen;
class ScanScreen;
class QTabWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(UserRole role, IFrameSource& source, ScanController& controller,
               IBarcodeReader& reader, ProductService& products, OrderService& orders,
               CheckoutService& checkout, QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    void onFrame(const Image& frame);
    void onCode(const std::string& code);

    IFrameSource& source_;
    ScanController& controller_;
    QTabWidget* tabs_;
    OrderScreen* orderScreen_;
    ScanScreen* scanScreen_;
};
