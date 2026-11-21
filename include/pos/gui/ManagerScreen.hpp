// ManagerScreen.hpp
// Dedicated workstation for Store Manager: product addition (scan or type) and inventory management.

#ifndef POS_MANAGER_SCREEN_HPP
#define POS_MANAGER_SCREEN_HPP

#include <QWidget>
#include <QString>
#include <vector>

#include "barcode/image.hpp"
#include "pos/domain/Product.hpp"

class QLineEdit;
class QDoubleSpinBox;
class QSpinBox;
class QComboBox;
class QTableWidget;
class QLabel;
class QPushButton;
class ProductService;
class IBarcodeReader;
class CameraView;

class ManagerScreen : public QWidget {
    Q_OBJECT

public:
    ManagerScreen(ProductService& products, IBarcodeReader& reader, QWidget* parent = nullptr);

    // Feeds video frame for barcode scanning during product creation
    void showFrame(const Image& frame);

    // Handles scanned barcode from camera
    void handleCode(const std::string& code);

    // Refreshes the inventory table
    void refreshInventory();

private slots:
    void onAddProductClicked();
    void onSearchChanged(const QString& query);
    void onScanToggleClicked();

private:
    ProductService& products_;
    IBarcodeReader& reader_;

    // Form inputs
    QSpinBox* idSpinBox_;
    QLineEdit* barcodeEdit_;
    QPushButton* scanCaptureBtn_;
    QLineEdit* acronymEdit_;
    QLineEdit* nameEdit_;
    QComboBox* unitCombo_;
    QDoubleSpinBox* priceSpinBox_;
    QDoubleSpinBox* taxSpinBox_;
    QDoubleSpinBox* discountSpinBox_;
    QDoubleSpinBox* quantitySpinBox_;
    QPushButton* addButton_;
    QLabel* statusLabel_;

    // Camera preview for scanning
    CameraView* cameraView_;
    bool isScanningActive_ = false;

    // Inventory Table
    QLineEdit* searchEdit_;
    QTableWidget* inventoryTable_;
    QPushButton* refreshButton_;

    void setupUi();
    void populateTable(const std::vector<Product>& products);
};

#endif  // POS_MANAGER_SCREEN_HPP
