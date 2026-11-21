// ManagerScreen.cpp
// Implementation of Manager workstation for product addition and inventory management.

#include "pos/gui/ManagerScreen.hpp"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

#include "barcode/validator.hpp"
#include "pos/common/Money.hpp"
#include "pos/common/PosError.hpp"
#include "pos/gui/CameraView.hpp"
#include "pos/scanning/IBarcodeReader.hpp"
#include "pos/services/ProductService.hpp"

ManagerScreen::ManagerScreen(ProductService& products, IBarcodeReader& reader, QWidget* parent)
    : QWidget(parent), products_(products), reader_(reader) {
    setupUi();
    refreshInventory();
}

void ManagerScreen::setupUi() {
    // ---------------- Left Panel: Add New Product Form ----------------
    QGroupBox* addGroup = new QGroupBox("Add New Product", this);

    idSpinBox_ = new QSpinBox(this);
    idSpinBox_->setRange(1, 999999);
    idSpinBox_->setValue(products_.nextProductId());

    barcodeEdit_ = new QLineEdit(this);
    barcodeEdit_->setPlaceholderText("Type 6-digit barcode or scan");

    scanCaptureBtn_ = new QPushButton("Scan from Camera", this);
    scanCaptureBtn_->setCheckable(true);
    connect(scanCaptureBtn_, &QPushButton::clicked, this, &ManagerScreen::onScanToggleClicked);

    QHBoxLayout* barcodeLayout = new QHBoxLayout();
    barcodeLayout->addWidget(barcodeEdit_, 2);
    barcodeLayout->addWidget(scanCaptureBtn_, 1);

    cameraView_ = new CameraView(this);
    cameraView_->setMinimumHeight(160);
    cameraView_->setMaximumHeight(200);
    cameraView_->hide(); // Hidden by default until "Scan from Camera" is toggled

    acronymEdit_ = new QLineEdit(this);
    acronymEdit_->setMaxLength(8);
    acronymEdit_->setPlaceholderText("e.g. TEA, BISCUIT (max 8 chars)");

    nameEdit_ = new QLineEdit(this);
    nameEdit_->setPlaceholderText("Full product description");

    unitCombo_ = new QComboBox(this);
    unitCombo_->addItem("Count (items)", "count");
    unitCombo_->addItem("Weight (kg)", "weight");

    priceSpinBox_ = new QDoubleSpinBox(this);
    priceSpinBox_->setRange(0.0, 99999.0);
    priceSpinBox_->setDecimals(2);
    priceSpinBox_->setPrefix("PKR ");

    taxSpinBox_ = new QDoubleSpinBox(this);
    taxSpinBox_->setRange(0.0, 9999.0);
    taxSpinBox_->setDecimals(2);
    taxSpinBox_->setPrefix("PKR ");

    discountSpinBox_ = new QDoubleSpinBox(this);
    discountSpinBox_->setRange(0.0, 9999.0);
    discountSpinBox_->setDecimals(2);
    discountSpinBox_->setPrefix("PKR ");

    quantitySpinBox_ = new QDoubleSpinBox(this);
    quantitySpinBox_->setRange(0.0, 99999.0);
    quantitySpinBox_->setDecimals(2);
    quantitySpinBox_->setValue(10.0);

    addButton_ = new QPushButton("Add Product & Generate Barcode", this);
    addButton_->setStyleSheet("background-color: #2563EB; color: white; font-weight: bold; padding: 10px; border-radius: 5px; border: none; font-size: 13px;");
    connect(addButton_, &QPushButton::clicked, this, &ManagerScreen::onAddProductClicked);

    scanCaptureBtn_->setStyleSheet(
        "QPushButton { background-color: #EFF6FF; color: #1D4ED8; font-weight: bold; "
        "border: 1.5px solid #3B82F6; border-radius: 5px; padding: 6px 12px; font-size: 12px; } "
        "QPushButton:hover { background-color: #DBEAFE; } "
        "QPushButton:checked { background-color: #2563EB; color: #FFFFFF; }");

    statusLabel_ = new QLabel(this);
    statusLabel_->setWordWrap(true);

    QFormLayout* formLayout = new QFormLayout();
    formLayout->addRow("Product ID:", idSpinBox_);
    formLayout->addRow("Barcode:", barcodeLayout);
    formLayout->addRow(cameraView_);
    formLayout->addRow("Acronym:", acronymEdit_);
    formLayout->addRow("Product Name:", nameEdit_);
    formLayout->addRow("Measurement Unit:", unitCombo_);
    formLayout->addRow("Unit Price:", priceSpinBox_);
    formLayout->addRow("Tax (PKR):", taxSpinBox_);
    formLayout->addRow("Discount (PKR):", discountSpinBox_);
    formLayout->addRow("Stock Quantity:", quantitySpinBox_);
    formLayout->addRow(addButton_);
    formLayout->addRow(statusLabel_);

    QString inputStyle = 
        "QLineEdit, QComboBox, QDoubleSpinBox, QSpinBox { "
        "border: 1.5px solid #94A3B8; border-radius: 5px; padding: 6px 8px; "
        "background-color: #FFFFFF; color: #000000; font-size: 13px; font-weight: 500; } "
        "QLineEdit:focus, QComboBox:focus, QDoubleSpinBox:focus, QSpinBox:focus { "
        "border: 2px solid #2563EB; color: #000000; } "
        "QLineEdit::placeholder { color: #64748B; font-weight: normal; }";

    addGroup->setStyleSheet(
        "QGroupBox { font-weight: bold; font-size: 14px; color: #0F172A; border: 1.5px solid #CBD5E1; "
        "border-radius: 8px; margin-top: 14px; padding-top: 16px; background-color: #FFFFFF; } "
        "QGroupBox::title { subcontrol-origin: margin; left: 14px; padding: 0 8px; color: #0F172A; background-color: #FFFFFF; } "
        "QLabel { color: #0F172A; font-weight: bold; font-size: 13px; } " + inputStyle
    );

    addGroup->setLayout(formLayout);

    // ---------------- Right Panel: Inventory Catalog ----------------
    QGroupBox* catalogGroup = new QGroupBox("Current Inventory Catalog", this);
    catalogGroup->setStyleSheet(
        "QGroupBox { font-weight: bold; font-size: 14px; color: #0F172A; border: 1.5px solid #CBD5E1; "
        "border-radius: 8px; margin-top: 14px; padding-top: 16px; background-color: #FFFFFF; } "
        "QGroupBox::title { subcontrol-origin: margin; left: 14px; padding: 0 8px; color: #0F172A; background-color: #FFFFFF; } "
        "QLabel { color: #0F172A; font-weight: bold; font-size: 13px; } " + inputStyle
    );

    searchEdit_ = new QLineEdit(this);
    searchEdit_->setPlaceholderText("Search by barcode, acronym, or name...");
    searchEdit_->setClearButtonEnabled(true);
    connect(searchEdit_, &QLineEdit::textChanged, this, &ManagerScreen::onSearchChanged);

    refreshButton_ = new QPushButton("Refresh", this);
    refreshButton_->setStyleSheet(
        "QPushButton { background-color: #F1F5F9; color: #0F172A; font-weight: bold; font-size: 13px; "
        "border: 1.5px solid #94A3B8; border-radius: 5px; padding: 6px 14px; } "
        "QPushButton:hover { background-color: #E2E8F0; }");
    connect(refreshButton_, &QPushButton::clicked, this, &ManagerScreen::refreshInventory);

    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchLayout->addWidget(searchEdit_, 3);
    searchLayout->addWidget(refreshButton_, 1);

    inventoryTable_ = new QTableWidget(0, 9, this);
    inventoryTable_->setHorizontalHeaderLabels(QStringList{
        "ID", "Barcode", "Acronym", "Name", "Unit", "Price (PKR)", "Tax", "Discount", "Available Qty"
    });
    inventoryTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    inventoryTable_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    inventoryTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    inventoryTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QVBoxLayout* catalogLayout = new QVBoxLayout();
    catalogLayout->addLayout(searchLayout);
    catalogLayout->addWidget(inventoryTable_);
    catalogGroup->setLayout(catalogLayout);

    // ---------------- Main Layout ----------------
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(addGroup, 1);
    mainLayout->addWidget(catalogGroup, 2);
}

void ManagerScreen::showFrame(const Image& frame) {
    if (isScanningActive_ && cameraView_->isVisible()) {
        cameraView_->showFrame(frame);
    }
}

void ManagerScreen::handleCode(const std::string& code) {
    if (isScanningActive_) {
        barcodeEdit_->setText(QString::fromStdString(code));
        statusLabel_->setStyleSheet("color: #10B981; font-weight: bold;");
        statusLabel_->setText("Scanned barcode: " + QString::fromStdString(code));
        // Turn off camera preview once barcode is captured
        isScanningActive_ = false;
        scanCaptureBtn_->setChecked(false);
        cameraView_->hide();
    }
}

void ManagerScreen::onScanToggleClicked() {
    isScanningActive_ = scanCaptureBtn_->isChecked();
    if (isScanningActive_) {
        cameraView_->show();
        statusLabel_->setStyleSheet("color: #3B82F6;");
        statusLabel_->setText("Hold barcode in camera view to capture code...");
    } else {
        cameraView_->hide();
        statusLabel_->clear();
    }
}

void ManagerScreen::onAddProductClicked() {
    statusLabel_->clear();

    const std::string barcode = barcodeEdit_->text().trimmed().toStdString();
    const std::string acronym = acronymEdit_->text().trimmed().toStdString();
    const std::string name = nameEdit_->text().trimmed().toStdString();
    const int id = idSpinBox_->value();
    const double price = priceSpinBox_->value();
    const double tax = taxSpinBox_->value();
    const double discount = discountSpinBox_->value();
    const double qty = quantitySpinBox_->value();
    const std::string unitStr = unitCombo_->currentData().toString().toStdString();

    if (barcode.empty()) {
        statusLabel_->setStyleSheet("color: #EF4444; font-weight: bold;");
        statusLabel_->setText("Please enter or scan a barcode.");
        return;
    }
    if (acronym.empty()) {
        statusLabel_->setStyleSheet("color: #EF4444; font-weight: bold;");
        statusLabel_->setText("Please enter an acronym (max 8 characters).");
        return;
    }
    if (name.empty()) {
        statusLabel_->setStyleSheet("color: #EF4444; font-weight: bold;");
        statusLabel_->setText("Please enter a product name.");
        return;
    }

    try {
        Product p;
        p.id = id;
        p.barcode = barcode;
        p.acronym = acronym;
        p.name = name;
        p.unit = (unitStr == "weight") ? MeasurementUnit::Weight : MeasurementUnit::Count;
        p.unitPrice = Money::fromDouble(price);
        p.tax = Money::fromDouble(tax);
        p.discount = Money::fromDouble(discount);
        p.availableQty = qty;
        p.barcodePath = "data/barcodes/" + barcode + ".bmp";

        products_.addProduct(p);

        statusLabel_->setStyleSheet("color: #10B981; font-weight: bold;");
        statusLabel_->setText("Product added successfully! Barcode asset generated.");

        // Clear input form for next entry
        barcodeEdit_->clear();
        acronymEdit_->clear();
        nameEdit_->clear();
        priceSpinBox_->setValue(0.0);
        taxSpinBox_->setValue(0.0);
        discountSpinBox_->setValue(0.0);
        quantitySpinBox_->setValue(10.0);
        idSpinBox_->setValue(products_.nextProductId());

        refreshInventory();
    } catch (const PosError& e) {
        statusLabel_->setStyleSheet("color: #EF4444; font-weight: bold;");
        statusLabel_->setText(QString("Error: ") + QString::fromUtf8(e.what()));
    } catch (const std::exception& e) {
        statusLabel_->setStyleSheet("color: #EF4444; font-weight: bold;");
        statusLabel_->setText(QString("Error: ") + QString::fromUtf8(e.what()));
    }
}

void ManagerScreen::refreshInventory() {
    std::vector<Product> all = products_.allProducts();
    populateTable(all);
    idSpinBox_->setValue(products_.nextProductId());
}

void ManagerScreen::onSearchChanged(const QString& query) {
    const std::string q = query.trimmed().toLower().toStdString();
    if (q.empty()) {
        refreshInventory();
        return;
    }

    std::vector<Product> filtered;
    for (const auto& p : products_.allProducts()) {
        std::string lowerName = p.name;
        for (char& c : lowerName) c = static_cast<char>(tolower(c));
        std::string lowerAcr = p.acronym;
        for (char& c : lowerAcr) c = static_cast<char>(tolower(c));

        if (p.barcode.find(q) != std::string::npos ||
            lowerName.find(q) != std::string::npos ||
            lowerAcr.find(q) != std::string::npos) {
            filtered.push_back(p);
        }
    }
    populateTable(filtered);
}

void ManagerScreen::populateTable(const std::vector<Product>& products) {
    inventoryTable_->setRowCount(static_cast<int>(products.size()));
    for (int i = 0; i < static_cast<int>(products.size()); ++i) {
        const auto& p = products[i];
        inventoryTable_->setItem(i, 0, new QTableWidgetItem(QString::number(p.id)));
        inventoryTable_->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(p.barcode)));
        inventoryTable_->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(p.acronym)));
        inventoryTable_->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(p.name)));
        inventoryTable_->setItem(i, 4, new QTableWidgetItem((p.unit == MeasurementUnit::Count) ? "count" : "weight"));
        inventoryTable_->setItem(i, 5, new QTableWidgetItem(QString::fromStdString(p.unitPrice.toString())));
        inventoryTable_->setItem(i, 6, new QTableWidgetItem(QString::fromStdString(p.tax.toString())));
        inventoryTable_->setItem(i, 7, new QTableWidgetItem(QString::fromStdString(p.discount.toString())));
        inventoryTable_->setItem(i, 8, new QTableWidgetItem(QString::number(p.availableQty, 'f', (p.unit == MeasurementUnit::Count ? 0 : 2))));
    }
}
