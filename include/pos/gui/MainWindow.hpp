// MainWindow.hpp                                      Owner: Person 1
// Primary application window orchestrating Cashier, Manager, and Scanner interfaces.

#ifndef POS_MAIN_WINDOW_HPP
#define POS_MAIN_WINDOW_HPP

#include <QMainWindow>
#include <string>

#include "pos/gui/RoleDialog.hpp"
#include "pos/scanning/IFrameSource.hpp"

class ScanController;
class IBarcodeReader;
class ProductService;
class OrderService;
class CheckoutService;
class CategoryService;
class SeasonService;
class ReportService;
class OrderScreen;
class ManagerScreen;
class ScanScreen;
class QTabWidget;
class QLabel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(UserRole role,
        IFrameSource& source,
        ScanController& controller,
        IBarcodeReader& reader,
        ProductService& products,
        OrderService& orders,
        CheckoutService& checkout,
        CategoryService& categories,
        SeasonService& seasons,
        ReportService& reports,
        const std::string& receiptsDir = "data/receipts",
        const std::string& shopName = "Store POS System",
        const std::string& managerPin = "1234",
        QWidget* parent = nullptr);
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
    std::string managerPin_;

    QLabel* roleBadge_ = nullptr;
    QTabWidget* tabs_ = nullptr;
    OrderScreen* orderScreen_ = nullptr;
    ManagerScreen* managerScreen_ = nullptr;
    ScanScreen* scanScreen_ = nullptr;
};

#endif // POS_MAIN_WINDOW_HPP