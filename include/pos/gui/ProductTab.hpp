// ProductTab.hpp                                   Owner: Person 5
// Manager UI tab for adding and editing products and barcode generation.

#ifndef POS_PRODUCT_TAB_HPP
#define POS_PRODUCT_TAB_HPP

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTabWidget>
#include <QLabel>
#include <vector>

#include "pos/services/ProductService.hpp"
#include "pos/services/CategoryService.hpp"

class ProductTab : public QWidget {
    Q_OBJECT

public:
    ProductTab(ProductService& productService,
               CategoryService& categoryService,
               QWidget* parent = nullptr);

    void refresh();

signals:
    void productsChanged();

private slots:
    void onCategoryChanged();
    void onGenerateBarcode();
    void onAddProduct();
    void onProductSelected(int row, int col);
    void onSaveProductEdit();

private:
    void setupUi();
    void refreshCategories();
    void refreshProducts();

    ProductService& productService_;
    CategoryService& categoryService_;

    // Add form
    QComboBox* categoryCombo_ = nullptr;
    QLineEdit* barcodeEdit_ = nullptr;
    QPushButton* generateBarcodeBtn_ = nullptr;
    QLineEdit* acronymEdit_ = nullptr;
    QLineEdit* nameEdit_ = nullptr;
    QComboBox* unitCombo_ = nullptr;
    QDoubleSpinBox* priceSpin_ = nullptr;
    QDoubleSpinBox* taxSpin_ = nullptr;
    QDoubleSpinBox* discountSpin_ = nullptr;
    QDoubleSpinBox* qtySpin_ = nullptr;
    QLabel* addStatusLabel_ = nullptr;

    // Edit form
    QLineEdit* editBarcode_ = nullptr;
    QLineEdit* editAcronym_ = nullptr;
    QLineEdit* editName_ = nullptr;
    QDoubleSpinBox* editPrice_ = nullptr;
    QDoubleSpinBox* editTax_ = nullptr;
    QDoubleSpinBox* editDiscount_ = nullptr;
    QDoubleSpinBox* editQty_ = nullptr;
    QPushButton* saveEditBtn_ = nullptr;
    QLabel* editStatusLabel_ = nullptr;
    int editingProductId_ = -1;

    // Catalog table and side panel
    QTableWidget* table_ = nullptr;
    QTabWidget* sideTabs_ = nullptr;
    std::vector<Product> currentProducts_;
};

#endif // POS_PRODUCT_TAB_HPP
