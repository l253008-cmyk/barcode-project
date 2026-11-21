// ProductTab.cpp                                   Owner: Person 5
// Manager UI tab for adding and editing products and barcode generation.

#include "pos/gui/ProductTab.hpp"
#include "pos/common/PosError.hpp"

#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QVBoxLayout>

ProductTab::ProductTab(ProductService& productService,
                       CategoryService& categoryService,
                       QWidget* parent)
    : QWidget(parent),
      productService_(productService),
      categoryService_(categoryService) {
    setupUi();
    refresh();
}

void ProductTab::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(14, 14, 14, 14);

    // Top section: Add Product and Edit Product side by side
    auto* formsLayout = new QHBoxLayout();
    formsLayout->setSpacing(12);

    // 1. Add Product Group
    auto* addBox = new QGroupBox("Add New Product (Barcode Auto-Prefixed)", this);
    addBox->setStyleSheet("QGroupBox { font-weight: bold; font-size: 13px; color: #1e293b; }");
    auto* addGrid = new QGridLayout(addBox);
    addGrid->setSpacing(8);

    addGrid->addWidget(new QLabel("Category:", addBox), 0, 0);
    categoryCombo_ = new QComboBox(addBox);
    categoryCombo_->setStyleSheet("QComboBox { padding: 4px 8px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; }");
    connect(categoryCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ProductTab::onCategoryChanged);
    addGrid->addWidget(categoryCombo_, 0, 1);

    addGrid->addWidget(new QLabel("Barcode:", addBox), 1, 0);
    auto* barcodeRow = new QHBoxLayout();
    barcodeEdit_ = new QLineEdit(addBox);
    barcodeEdit_->setReadOnly(true);
    barcodeEdit_->setStyleSheet("QLineEdit { padding: 4px 8px; background: #f1f5f9; color: #0f172a; border: 1px solid #cbd5e1; border-radius: 4px; font-weight: bold; }");
    barcodeRow->addWidget(barcodeEdit_);
    generateBarcodeBtn_ = new QPushButton("Refresh", addBox);
    generateBarcodeBtn_->setStyleSheet("QPushButton { padding: 4px 10px; background: #64748b; color: #ffffff; border-radius: 4px; border: none; }");
    connect(generateBarcodeBtn_, &QPushButton::clicked, this, &ProductTab::onGenerateBarcode);
    barcodeRow->addWidget(generateBarcodeBtn_);
    addGrid->addLayout(barcodeRow, 1, 1);

    addGrid->addWidget(new QLabel("Name:", addBox), 2, 0);
    nameEdit_ = new QLineEdit(addBox);
    nameEdit_->setPlaceholderText("Product Name");
    nameEdit_->setStyleSheet("QLineEdit { padding: 4px 8px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; }");
    addGrid->addWidget(nameEdit_, 2, 1);

    addGrid->addWidget(new QLabel("Acronym:", addBox), 3, 0);
    acronymEdit_ = new QLineEdit(addBox);
    acronymEdit_->setPlaceholderText("Short Code (e.g. TEA)");
    acronymEdit_->setMaxLength(8);
    acronymEdit_->setStyleSheet("QLineEdit { padding: 4px 8px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; }");
    addGrid->addWidget(acronymEdit_, 3, 1);

    addGrid->addWidget(new QLabel("Unit:", addBox), 4, 0);
    unitCombo_ = new QComboBox(addBox);
    unitCombo_->addItem("Count (Pack/Piece)", static_cast<int>(MeasurementUnit::Count));
    unitCombo_->addItem("Weight (kg)", static_cast<int>(MeasurementUnit::Weight));
    unitCombo_->setStyleSheet("QComboBox { padding: 4px 8px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; }");
    addGrid->addWidget(unitCombo_, 4, 1);

    addGrid->addWidget(new QLabel("Price (PKR):", addBox), 5, 0);
    priceSpin_ = new QDoubleSpinBox(addBox);
    priceSpin_->setRange(0.01, 999999.0);
    priceSpin_->setValue(100.0);
    priceSpin_->setDecimals(2);
    priceSpin_->setStyleSheet("QDoubleSpinBox { padding: 4px 8px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; }");
    addGrid->addWidget(priceSpin_, 5, 1);

    addGrid->addWidget(new QLabel("Tax (PKR):", addBox), 6, 0);
    taxSpin_ = new QDoubleSpinBox(addBox);
    taxSpin_->setRange(0.0, 99999.0);
    taxSpin_->setValue(0.0);
    taxSpin_->setDecimals(2);
    taxSpin_->setStyleSheet("QDoubleSpinBox { padding: 4px 8px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; }");
    addGrid->addWidget(taxSpin_, 6, 1);

    addGrid->addWidget(new QLabel("Discount (PKR):", addBox), 7, 0);
    discountSpin_ = new QDoubleSpinBox(addBox);
    discountSpin_->setRange(0.0, 99999.0);
    discountSpin_->setValue(0.0);
    discountSpin_->setDecimals(2);
    discountSpin_->setStyleSheet("QDoubleSpinBox { padding: 4px 8px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; }");
    addGrid->addWidget(discountSpin_, 7, 1);

    addGrid->addWidget(new QLabel("Initial Stock:", addBox), 8, 0);
    qtySpin_ = new QDoubleSpinBox(addBox);
    qtySpin_->setRange(0.0, 99999.0);
    qtySpin_->setValue(50.0);
    qtySpin_->setStyleSheet("QDoubleSpinBox { padding: 4px 8px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; }");
    addGrid->addWidget(qtySpin_, 8, 1);

    auto* addBtn = new QPushButton("Add Product", addBox);
    addBtn->setStyleSheet("QPushButton { padding: 8px; background: #2563eb; color: #ffffff; font-weight: bold; border-radius: 4px; border: none; } QPushButton:hover { background: #1d4ed8; }");
    connect(addBtn, &QPushButton::clicked, this, &ProductTab::onAddProduct);
    addGrid->addWidget(addBtn, 9, 0, 1, 2);

    addStatusLabel_ = new QLabel(addBox);
    addStatusLabel_->setStyleSheet("font-size: 11px; font-weight: 500;");
    addStatusLabel_->setVisible(false);
    addGrid->addWidget(addStatusLabel_, 10, 0, 1, 2);

    formsLayout->addWidget(addBox);

    // 2. Edit Product Group
    auto* editBox = new QGroupBox("Edit Selected Product (Click table row to edit)", this);
    editBox->setStyleSheet("QGroupBox { font-weight: bold; font-size: 13px; color: #1e293b; }");
    auto* editGrid = new QGridLayout(editBox);
    editGrid->setSpacing(8);

    editGrid->addWidget(new QLabel("Barcode (Fixed):", editBox), 0, 0);
    editBarcode_ = new QLineEdit(editBox);
    editBarcode_->setReadOnly(true);
    editBarcode_->setStyleSheet("QLineEdit { padding: 4px 8px; background: #f1f5f9; color: #64748b; border: 1px solid #cbd5e1; border-radius: 4px; }");
    editGrid->addWidget(editBarcode_, 0, 1);

    editGrid->addWidget(new QLabel("Acronym (Fixed):", editBox), 1, 0);
    editAcronym_ = new QLineEdit(editBox);
    editAcronym_->setReadOnly(true);
    editAcronym_->setStyleSheet("QLineEdit { padding: 4px 8px; background: #f1f5f9; color: #64748b; border: 1px solid #cbd5e1; border-radius: 4px; }");
    editGrid->addWidget(editAcronym_, 1, 1);

    editGrid->addWidget(new QLabel("Name (Editable):", editBox), 2, 0);
    editName_ = new QLineEdit(editBox);
    editName_->setStyleSheet("QLineEdit { padding: 4px 8px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; }");
    editGrid->addWidget(editName_, 2, 1);

    editGrid->addWidget(new QLabel("Price (PKR):", editBox), 3, 0);
    editPrice_ = new QDoubleSpinBox(editBox);
    editPrice_->setRange(0.01, 999999.0);
    editPrice_->setDecimals(2);
    editPrice_->setStyleSheet("QDoubleSpinBox { padding: 4px 8px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; }");
    editGrid->addWidget(editPrice_, 3, 1);

    editGrid->addWidget(new QLabel("Tax (PKR):", editBox), 4, 0);
    editTax_ = new QDoubleSpinBox(editBox);
    editTax_->setRange(0.0, 99999.0);
    editTax_->setDecimals(2);
    editTax_->setStyleSheet("QDoubleSpinBox { padding: 4px 8px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; }");
    editGrid->addWidget(editTax_, 4, 1);

    editGrid->addWidget(new QLabel("Discount (PKR):", editBox), 5, 0);
    editDiscount_ = new QDoubleSpinBox(editBox);
    editDiscount_->setRange(0.0, 99999.0);
    editDiscount_->setDecimals(2);
    editDiscount_->setStyleSheet("QDoubleSpinBox { padding: 4px 8px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; }");
    editGrid->addWidget(editDiscount_, 5, 1);

    editGrid->addWidget(new QLabel("Stock Qty:", editBox), 6, 0);
    editQty_ = new QDoubleSpinBox(editBox);
    editQty_->setRange(0.0, 99999.0);
    editQty_->setStyleSheet("QDoubleSpinBox { padding: 4px 8px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; }");
    editGrid->addWidget(editQty_, 6, 1);

    saveEditBtn_ = new QPushButton("Save Changes", editBox);
    saveEditBtn_->setEnabled(false);
    saveEditBtn_->setStyleSheet("QPushButton { padding: 8px; background: #059669; color: #ffffff; font-weight: bold; border-radius: 4px; border: none; } QPushButton:hover { background: #047857; } QPushButton:disabled { background: #94a3b8; }");
    connect(saveEditBtn_, &QPushButton::clicked, this, &ProductTab::onSaveProductEdit);
    editGrid->addWidget(saveEditBtn_, 7, 0, 1, 2);

    editStatusLabel_ = new QLabel(editBox);
    editStatusLabel_->setStyleSheet("font-size: 11px; font-weight: 500;");
    editStatusLabel_->setVisible(false);
    editGrid->addWidget(editStatusLabel_, 8, 0, 1, 2);

    formsLayout->addWidget(editBox);
    mainLayout->addLayout(formsLayout);

    // Products table
    table_ = new QTableWidget(this);
    table_->setColumnCount(9);
    table_->setHorizontalHeaderLabels({"ID", "Cat", "Barcode", "Acronym", "Name", "Unit", "Price", "Tax", "Stock"});
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(8, QHeaderView::ResizeToContents);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setStyleSheet(
        "QTableWidget { background-color: #ffffff; color: #0f172a; border: 1px solid #e2e8f0; border-radius: 6px; }"
        "QHeaderView::section { background-color: #f1f5f9; color: #334155; font-weight: bold; padding: 6px; border: none; }"
    );
    connect(table_, &QTableWidget::cellClicked, this, &ProductTab::onProductSelected);
    mainLayout->addWidget(table_);
}

void ProductTab::refresh() {
    refreshCategories();
    refreshProducts();
}

void ProductTab::refreshCategories() {
    QString currentCat = categoryCombo_->currentData().toString();
    categoryCombo_->blockSignals(true);
    categoryCombo_->clear();

    auto categories = categoryService_.all();
    for (const auto& cat : categories) {
        QString text = QString::fromStdString(cat.id + " - " + cat.name + " (" + cat.acronym + ")");
        categoryCombo_->addItem(text, QString::fromStdString(cat.id));
    }

    int idx = categoryCombo_->findData(currentCat);
    if (idx >= 0) {
        categoryCombo_->setCurrentIndex(idx);
    }
    categoryCombo_->blockSignals(false);

    onCategoryChanged();
}

void ProductTab::onCategoryChanged() {
    onGenerateBarcode();
}

void ProductTab::onGenerateBarcode() {
    if (categoryCombo_->count() == 0) {
        barcodeEdit_->setText("");
        return;
    }
    std::string catId = categoryCombo_->currentData().toString().toStdString();
    try {
        std::string generated = productService_.generateBarcode(catId);
        barcodeEdit_->setText(QString::fromStdString(generated));
    } catch (const std::exception&) {
        barcodeEdit_->setText("");
    }
}

void ProductTab::refreshProducts() {
    currentProducts_ = productService_.allProducts();
    table_->setRowCount(static_cast<int>(currentProducts_.size()));

    for (int i = 0; i < static_cast<int>(currentProducts_.size()); ++i) {
        const auto& p = currentProducts_[i];

        auto* idItem = new QTableWidgetItem(QString::number(p.id));
        idItem->setTextAlignment(Qt::AlignCenter);
        table_->setItem(i, 0, idItem);

        std::string displayCat = p.categoryId.empty() ? (p.barcode.size() >= 2 ? p.barcode.substr(0, 2) : "") : p.categoryId;
        auto* catItem = new QTableWidgetItem(QString::fromStdString(displayCat));
        catItem->setTextAlignment(Qt::AlignCenter);
        table_->setItem(i, 1, catItem);

        auto* bcItem = new QTableWidgetItem(QString::fromStdString(p.barcode));
        bcItem->setTextAlignment(Qt::AlignCenter);
        table_->setItem(i, 2, bcItem);

        auto* acroItem = new QTableWidgetItem(QString::fromStdString(p.acronym));
        acroItem->setTextAlignment(Qt::AlignCenter);
        table_->setItem(i, 3, acroItem);

        table_->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(p.name)));

        auto* unitItem = new QTableWidgetItem(QString::fromStdString(unitToText(p.unit)));
        unitItem->setTextAlignment(Qt::AlignCenter);
        table_->setItem(i, 5, unitItem);

        auto* priceItem = new QTableWidgetItem(QString::fromStdString(p.unitPrice.toString()));
        priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        table_->setItem(i, 6, priceItem);

        auto* taxItem = new QTableWidgetItem(QString::fromStdString(p.tax.toString()));
        taxItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        table_->setItem(i, 7, taxItem);

        auto* stockItem = new QTableWidgetItem(QString::fromStdString(quantityToText(p.availableQty)));
        stockItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        table_->setItem(i, 8, stockItem);
    }
}

void ProductTab::onAddProduct() {
    if (categoryCombo_->count() == 0) {
        addStatusLabel_->setText("Please configure at least one category first.");
        addStatusLabel_->setStyleSheet("color: #dc2626;");
        addStatusLabel_->setVisible(true);
        return;
    }

    std::string catId = categoryCombo_->currentData().toString().toStdString();
    std::string name = nameEdit_->text().trimmed().toStdString();
    std::string acronym = acronymEdit_->text().trimmed().toStdString();
    auto unit = static_cast<MeasurementUnit>(unitCombo_->currentData().toInt());
    Money price = Money::fromDouble(priceSpin_->value());
    Money tax = Money::fromDouble(taxSpin_->value());
    Money discount = Money::fromDouble(discountSpin_->value());
    double qty = qtySpin_->value();

    try {
        productService_.addProduct(catId, name, acronym, unit, price, tax, discount, qty);
        addStatusLabel_->setText("Product added successfully!");
        addStatusLabel_->setStyleSheet("color: #16a34a; font-weight: bold;");
        addStatusLabel_->setVisible(true);

        nameEdit_->clear();
        acronymEdit_->clear();

        refreshProducts();
        onGenerateBarcode();
        emit productsChanged();
    } catch (const PosError& e) {
        addStatusLabel_->setText(QString::fromStdString(e.what()));
        addStatusLabel_->setStyleSheet("color: #dc2626; font-weight: bold;");
        addStatusLabel_->setVisible(true);
    }
}

void ProductTab::onProductSelected(int row, int /*col*/) {
    if (row < 0 || row >= static_cast<int>(currentProducts_.size())) {
        return;
    }
    const auto& p = currentProducts_[row];
    editingProductId_ = p.id;
    editBarcode_->setText(QString::fromStdString(p.barcode));
    editAcronym_->setText(QString::fromStdString(p.acronym));
    editName_->setText(QString::fromStdString(p.name));
    editPrice_->setValue(p.unitPrice.paisa() / 100.0);
    editTax_->setValue(p.tax.paisa() / 100.0);
    editDiscount_->setValue(p.discount.paisa() / 100.0);
    editQty_->setValue(p.availableQty);

    saveEditBtn_->setEnabled(true);
    editStatusLabel_->setVisible(false);
}

void ProductTab::onSaveProductEdit() {
    if (editingProductId_ < 0) return;

    // Find original product
    auto it = std::find_if(currentProducts_.begin(), currentProducts_.end(),
                           [this](const Product& p) { return p.id == editingProductId_; });
    if (it == currentProducts_.end()) return;

    Product updated = *it;
    updated.name = editName_->text().trimmed().toStdString();
    updated.unitPrice = Money::fromDouble(editPrice_->value());
    updated.tax = Money::fromDouble(editTax_->value());
    updated.discount = Money::fromDouble(editDiscount_->value());
    updated.availableQty = editQty_->value();

    try {
        productService_.updateProduct(updated);
        editStatusLabel_->setText("Changes saved successfully!");
        editStatusLabel_->setStyleSheet("color: #16a34a; font-weight: bold;");
        editStatusLabel_->setVisible(true);

        refreshProducts();
        emit productsChanged();
    } catch (const PosError& e) {
        editStatusLabel_->setText(QString::fromStdString(e.what()));
        editStatusLabel_->setStyleSheet("color: #dc2626; font-weight: bold;");
        editStatusLabel_->setVisible(true);
    }
}
