// OrderScreen.hpp                                      Owner: Person 1
// Cashier workstation: barcode scanning, fast checkout, hotkeys (F2-F12),
// hold/resume, and order editing with manager approval.

#ifndef POS_ORDER_SCREEN_HPP
#define POS_ORDER_SCREEN_HPP

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
    OrderScreen(OrderService& orders,
                ProductService& products,
                CheckoutService& checkout,
                const std::string& receiptsDir = "data/receipts",
                const std::string& shopName = "Store POS System",
                const std::string& managerPin = "1234",
                QWidget* parent = nullptr);

    void showFrame(const Image& frame);
    void handleCode(const std::string& code);

public slots:
    void newOrder();
    void holdOrder();
    void resumeOrder();
    void openEditOrder();
    void generateBill();

private slots:
    void describeSelected();
    void viewHeldOrders();
    void resumeSpecificOrder(const std::string& orderId);
    void removeSelected();
    void incrementSelected();
    void decrementSelected();
    void editQuantitySelected();
    void onPaymentInputsChanged();
    void addManualBarcode();

private:
    void updateHoldButtonState();
    void refreshTable();
    void clearForm();
    void setStatus(const QString& text, bool isError);
    std::string selectedBarcode() const;
    void recalculateTotals();

    OrderService& orders_;
    ProductService& products_;
    CheckoutService& checkout_;
    std::string receiptsDir_;
    std::string shopName_;
    std::string managerPin_;

    CameraView* camera_ = nullptr;
    QLineEdit* manualBarcodeInput_ = nullptr;
    ProductDetailsWidget* details_ = nullptr;
    QTableWidget* table_ = nullptr;
    QDoubleSpinBox* quantity_ = nullptr;
    QLineEdit* customerEdit_ = nullptr;
    QLineEdit* contactEdit_ = nullptr;
    QComboBox* paymentCombo_ = nullptr;
    QLineEdit* merchantEdit_ = nullptr;
    QLabel* status_ = nullptr;

    // Fast order action buttons
    QPushButton* newOrderButton_ = nullptr;
    QPushButton* holdButton_ = nullptr;
    QPushButton* viewHeldButton_ = nullptr;
    QPushButton* editOrderButton_ = nullptr;
    QPushButton* billButton_ = nullptr;

    // Payment & discount inputs
    QLabel* totalTaxLabel_ = nullptr;
    QLabel* totalDiscountLabel_ = nullptr;
    QDoubleSpinBox* specialDiscountSpin_ = nullptr;
    QDoubleSpinBox* amountReceivedSpin_ = nullptr;
    QLabel* payableAmountLabel_ = nullptr;
    QLabel* balanceReturnedLabel_ = nullptr;

    std::string lastCode_;
};

#endif // POS_ORDER_SCREEN_HPP
