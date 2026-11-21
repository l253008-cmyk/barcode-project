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
class ManagerScreen;
class ScanScreen;
class QTabWidget;
class QLabel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(UserRole role, IFrameSource& source, ScanController& controller,
               IBarcodeReader& reader, ProductService& products, OrderService& orders,
               CheckoutService& checkout, QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void switchRole();
    void onTabChanged(int index);

private:
    void onFrame(const Image& frame);
    void onCode(const std::string& code);
    void updateRoleDisplay();
    void applyStyleSheet();

    UserRole currentRole_;
    IFrameSource& source_;
    ScanController& controller_;

    QLabel* roleBadge_ = nullptr;
    QTabWidget* tabs_ = nullptr;
    OrderScreen* orderScreen_ = nullptr;
    ManagerScreen* managerScreen_ = nullptr;
    ScanScreen* scanScreen_ = nullptr;
};
