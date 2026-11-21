#pragma once

#include <QString>
#include <QWidget>
#include <string>

#include "pos/scanning/IFrameSource.hpp"

class CameraView;
class ProductDetailsWidget;
class OrderService;
class ProductService;
class CheckoutService;
class QTableWidget;
class QLineEdit;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;

class OrderScreen : public QWidget {
    Q_OBJECT

public:
    OrderScreen(OrderService& orders, ProductService& products, CheckoutService& checkout,
                QWidget* parent = nullptr);

    void showFrame(const Image& frame);
    void handleCode(const std::string& code);

private slots:
    void describeSelected();
    void removeSelected();
    void incrementSelected();
    void decrementSelected();
    void editQuantitySelected();
    void holdOrder();
    void resumeOrder();
    void generateBill();

private:
    void updateHoldButtonState();
    void refreshTable();
    void clearForm();
    void setStatus(const QString& text, bool isError);
    std::string selectedBarcode() const;

    OrderService& orders_;
    ProductService& products_;
    CheckoutService& checkout_;

    CameraView* camera_;
    ProductDetailsWidget* details_;
    QTableWidget* table_;
    QDoubleSpinBox* quantity_;
    QLineEdit* customerEdit_;
    QLineEdit* contactEdit_;
    QComboBox* paymentCombo_;
    QLineEdit* merchantEdit_;
    QLabel* status_;
    QPushButton* holdButton_ = nullptr;
    QPushButton* resumeButton_ = nullptr;
    std::string lastCode_;
};
