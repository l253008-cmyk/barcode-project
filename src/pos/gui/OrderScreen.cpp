// OrderScreen.cpp                                      Owner: Person 1
// Cashier workstation: barcode scanning, fast checkout, hotkeys (F2-F12),
// hold/resume, and order editing with manager approval.

#include "pos/gui/OrderScreen.hpp"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QTableWidget>
#include <QVBoxLayout>

#include "barcode/error.hpp"
#include "pos/common/PosError.hpp"
#include "pos/domain/Bill.hpp"
#include "pos/domain/Order.hpp"
#include "pos/gui/BillDialog.hpp"
#include "pos/gui/CameraView.hpp"
#include "pos/gui/HeldOrdersDialog.hpp"
#include "pos/gui/ManagerApprovalDialog.hpp"
#include "pos/gui/ProductDetailsWidget.hpp"
#include "pos/services/CheckoutService.hpp"
#include "pos/services/OrderService.hpp"
#include "pos/services/ProductService.hpp"

static QTableWidgetItem* makeItem(const QString& text) {
    auto* item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
}

OrderScreen::OrderScreen(OrderService& orders,
                         ProductService& products,
                         CheckoutService& checkout,
                         const std::string& receiptsDir,
                         const std::string& shopName,
                         const std::string& managerPin,
                         QWidget* parent)
    : QWidget(parent),
      orders_(orders),
      products_(products),
      checkout_(checkout),
      receiptsDir_(receiptsDir),
      shopName_(shopName),
      managerPin_(managerPin) {
    camera_ = new CameraView(this);
    details_ = new ProductDetailsWidget(this);

    quantity_ = new QDoubleSpinBox(this);
    quantity_->setRange(0.01, 9999.0);
    quantity_->setDecimals(2);
    quantity_->setValue(1.0);
    quantity_->setStyleSheet("QDoubleSpinBox { padding: 4px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; }");

    table_ = new QTableWidget(0, 5, this);
    table_->setHorizontalHeaderLabels(
        QStringList{"Barcode", "Product", "Unit price", "Quantity", "Amount"});
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setStyleSheet(
        "QTableWidget { background-color: #ffffff; color: #0f172a; border: 1px solid #e2e8f0; border-radius: 6px; }"
        "QHeaderView::section { background-color: #f1f5f9; color: #334155; font-weight: bold; padding: 6px; border: none; }"
    );

    customerEdit_ = new QLineEdit(this);
    customerEdit_->setPlaceholderText("Customer Name (defaults to Walk-in)");
    customerEdit_->setStyleSheet("QLineEdit { padding: 5px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; }");

    contactEdit_ = new QLineEdit(this);
    contactEdit_->setPlaceholderText("Contact Phone/Email");
    contactEdit_->setStyleSheet("QLineEdit { padding: 5px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; }");

    paymentCombo_ = new QComboBox(this);
    paymentCombo_->addItems(QStringList{"cash", "card", "wallet"});
    paymentCombo_->setStyleSheet("QComboBox { padding: 5px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; }");

    merchantEdit_ = new QLineEdit(this);
    merchantEdit_->setPlaceholderText("Merchant (for Card/Wallet)");
    merchantEdit_->setStyleSheet("QLineEdit { padding: 5px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; }");

    status_ = new QLabel("Scan a product or click New Order.", this);
    status_->setWordWrap(true);
    status_->setStyleSheet("font-size: 12px; font-weight: 500; color: #334155;");

    // Item manipulation buttons row
    auto* incBtn = new QPushButton("➕ 1", this);
    auto* decBtn = new QPushButton("➖ 1", this);
    auto* editQtyBtn = new QPushButton("✏️ Edit Qty", this);
    auto* descriptionButton = new QPushButton("ℹ️ Info", this);
    auto* removeButton = new QPushButton("🗑️ Remove", this);

    incBtn->setStyleSheet("QPushButton { background-color: #eff6ff; color: #1d4ed8; font-weight: bold; border: 1px solid #3b82f6; border-radius: 4px; padding: 6px 12px; } QPushButton:hover { background-color: #dbeafe; }");
    decBtn->setStyleSheet("QPushButton { background-color: #fef2f2; color: #b91c1c; font-weight: bold; border: 1px solid #ef4444; border-radius: 4px; padding: 6px 12px; } QPushButton:hover { background-color: #fee2e2; }");
    editQtyBtn->setStyleSheet("QPushButton { background-color: #f8fafc; color: #0f172a; font-weight: bold; border: 1px solid #64748b; border-radius: 4px; padding: 6px 12px; } QPushButton:hover { background-color: #e2e8f0; }");
    descriptionButton->setStyleSheet("QPushButton { background-color: #f1f5f9; color: #0f172a; font-weight: 600; border: 1px solid #94a3b8; border-radius: 4px; padding: 6px 12px; } QPushButton:hover { background-color: #e2e8f0; }");
    removeButton->setStyleSheet("QPushButton { background-color: #dc2626; color: #ffffff; font-weight: bold; border: none; border-radius: 4px; padding: 6px 12px; } QPushButton:hover { background-color: #b91c1c; }");

    connect(incBtn, &QPushButton::clicked, this, &OrderScreen::incrementSelected);
    connect(decBtn, &QPushButton::clicked, this, &OrderScreen::decrementSelected);
    connect(editQtyBtn, &QPushButton::clicked, this, &OrderScreen::editQuantitySelected);
    connect(descriptionButton, &QPushButton::clicked, this, &OrderScreen::describeSelected);
    connect(removeButton, &QPushButton::clicked, this, &OrderScreen::removeSelected);

    auto* itemButtons = new QHBoxLayout();
    itemButtons->addWidget(incBtn);
    itemButtons->addWidget(decBtn);
    itemButtons->addWidget(editQtyBtn);
    itemButtons->addWidget(descriptionButton);
    itemButtons->addWidget(removeButton);

    // Payment & Settlement Box with Tax and Discount preview
    auto* paymentBox = new QGroupBox("Payment & Settlement", this);
    paymentBox->setStyleSheet("QGroupBox { font-weight: bold; font-size: 13px; color: #1e293b; }");
    auto* paymentLayout = new QGridLayout(paymentBox);
    paymentLayout->setSpacing(8);

    paymentLayout->addWidget(new QLabel("Total Tax:", paymentBox), 0, 0);
    totalTaxLabel_ = new QLabel("0.00 PKR", paymentBox);
    totalTaxLabel_->setStyleSheet("font-size: 13px; font-weight: bold; color: #475569;");
    paymentLayout->addWidget(totalTaxLabel_, 0, 1);

    paymentLayout->addWidget(new QLabel("Total Discount:", paymentBox), 0, 2);
    totalDiscountLabel_ = new QLabel("0.00 PKR", paymentBox);
    totalDiscountLabel_->setStyleSheet("font-size: 13px; font-weight: bold; color: #dc2626;");
    paymentLayout->addWidget(totalDiscountLabel_, 0, 3);

    paymentLayout->addWidget(new QLabel("Special Disc (% max 8):", paymentBox), 1, 0);
    specialDiscountSpin_ = new QDoubleSpinBox(paymentBox);
    specialDiscountSpin_->setRange(0.0, 8.0);
    specialDiscountSpin_->setSingleStep(0.5);
    specialDiscountSpin_->setSuffix(" %");
    connect(specialDiscountSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &OrderScreen::onPaymentInputsChanged);
    paymentLayout->addWidget(specialDiscountSpin_, 1, 1);

    paymentLayout->addWidget(new QLabel("Payable Amount:", paymentBox), 1, 2);
    payableAmountLabel_ = new QLabel("0.00 PKR", paymentBox);
    payableAmountLabel_->setStyleSheet("font-size: 14px; font-weight: bold; color: #2563eb;");
    paymentLayout->addWidget(payableAmountLabel_, 1, 3);

    paymentLayout->addWidget(new QLabel("Amount Received:", paymentBox), 2, 0);
    amountReceivedSpin_ = new QDoubleSpinBox(paymentBox);
    amountReceivedSpin_->setRange(0.0, 9999999.0);
    amountReceivedSpin_->setDecimals(2);
    connect(amountReceivedSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &OrderScreen::onPaymentInputsChanged);
    paymentLayout->addWidget(amountReceivedSpin_, 2, 1);

    paymentLayout->addWidget(new QLabel("Change / Balance:", paymentBox), 2, 2);
    balanceReturnedLabel_ = new QLabel("0.00 PKR", paymentBox);
    balanceReturnedLabel_->setStyleSheet("font-size: 14px; font-weight: bold; color: #16a34a;");
    paymentLayout->addWidget(balanceReturnedLabel_, 2, 3);

    // Order action buttons row
    newOrderButton_ = new QPushButton("🆕 New Order", this);
    holdButton_ = new QPushButton("⏸️ Hold Order", this);
    viewHeldButton_ = new QPushButton("📋 View Held Orders", this);
    editOrderButton_ = new QPushButton("📂 Edit Order", this);
    billButton_ = new QPushButton("🧾 Generate Bill", this);

    newOrderButton_->setStyleSheet("QPushButton { background-color: #64748b; color: #ffffff; font-weight: bold; padding: 8px 12px; border-radius: 6px; border: none; } QPushButton:hover { background-color: #475569; }");
    holdButton_->setStyleSheet("QPushButton { background-color: #d97706; color: #ffffff; font-weight: bold; padding: 8px 12px; border-radius: 6px; border: none; } QPushButton:hover { background-color: #b45309; }");
    viewHeldButton_->setStyleSheet("QPushButton { background-color: #4f46e5; color: #ffffff; font-weight: bold; padding: 8px 12px; border-radius: 6px; border: none; } QPushButton:hover { background-color: #4338ca; } QPushButton:disabled { background-color: #e2e8f0; color: #94a3b8; }");
    editOrderButton_->setStyleSheet("QPushButton { background-color: #0284c7; color: #ffffff; font-weight: bold; padding: 8px 12px; border-radius: 6px; border: none; } QPushButton:hover { background-color: #0369a1; }");
    billButton_->setStyleSheet("QPushButton { background-color: #10b981; color: #ffffff; font-weight: bold; padding: 8px 16px; border-radius: 6px; border: none; } QPushButton:hover { background-color: #059669; }");

    connect(newOrderButton_, &QPushButton::clicked, this, &OrderScreen::newOrder);
    connect(holdButton_, &QPushButton::clicked, this, &OrderScreen::holdOrder);
    connect(viewHeldButton_, &QPushButton::clicked, this, &OrderScreen::viewHeldOrders);
    connect(editOrderButton_, &QPushButton::clicked, this, &OrderScreen::openEditOrder);
    connect(billButton_, &QPushButton::clicked, this, &OrderScreen::generateBill);

    auto* orderButtons = new QHBoxLayout();
    orderButtons->addWidget(newOrderButton_);
    orderButtons->addWidget(holdButton_);
    orderButtons->addWidget(viewHeldButton_);
    orderButtons->addWidget(editOrderButton_);
    orderButtons->addWidget(billButton_);

    // Manual typing space under scanning box and above quantity
    manualBarcodeInput_ = new QLineEdit(this);
    manualBarcodeInput_->setPlaceholderText("Enter barcode & press Enter");
    manualBarcodeInput_->setStyleSheet("QLineEdit { padding: 6px; border: 1.5px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; font-weight: 500; }");
    connect(manualBarcodeInput_, &QLineEdit::returnPressed, this, &OrderScreen::addManualBarcode);

    auto* manualForm = new QFormLayout();
    manualForm->addRow("Manual barcode:", manualBarcodeInput_);

    auto* quantityForm = new QFormLayout();
    quantityForm->addRow("Quantity per scan:", quantity_);

    auto* left = new QVBoxLayout();
    left->addWidget(camera_, 1);
    left->addLayout(manualForm);
    left->addLayout(quantityForm);
    left->addWidget(details_, 1);

    auto* customerForm = new QFormLayout();
    customerForm->addRow("Customer name:", customerEdit_);
    customerForm->addRow("Contact:", contactEdit_);
    customerForm->addRow("Payment mode:", paymentCombo_);
    customerForm->addRow("Merchant:", merchantEdit_);

    auto* right = new QVBoxLayout();
    right->addWidget(table_, 1);
    right->addLayout(itemButtons);
    right->addLayout(customerForm);
    right->addWidget(paymentBox);
    right->addLayout(orderButtons);
    right->addWidget(status_);

    auto* layout = new QHBoxLayout(this);
    layout->addLayout(left, 1);
    layout->addLayout(right, 2);

    orders_.start();
    refreshTable();
    updateHoldButtonState();
}

void OrderScreen::addManualBarcode() {
    QString code = manualBarcodeInput_->text().trimmed();
    if (code.isEmpty()) return;
    handleCode(code.toStdString());
    manualBarcodeInput_->clear();
}

void OrderScreen::showFrame(const Image& frame) {
    camera_->showFrame(frame);
}

void OrderScreen::handleCode(const std::string& code) {
    lastCode_ = code;
    try {
        orders_.addItem(code, quantity_->value());
        refreshTable();
        recalculateTotals();
        setStatus("Added product " + QString::fromStdString(code), false);
    } catch (const PosError& error) {
        setStatus(QString::fromUtf8(error.what()), true);
    } catch (const BarcodeError& error) {
        setStatus(QString::fromUtf8(error.what()), true);
    }
}

std::string OrderScreen::selectedBarcode() const {
    int row = table_->currentRow();
    if (row >= 0 && table_->item(row, 0) != nullptr) {
        return table_->item(row, 0)->text().toStdString();
    }
    return lastCode_;
}

void OrderScreen::describeSelected() {
    std::string code = selectedBarcode();
    if (code.empty()) {
        setStatus("Select an item or scan a product first.", true);
        return;
    }
    try {
        details_->showDescription(QString::fromStdString(products_.describeByBarcode(code)));
    } catch (const PosError& error) {
        details_->showMessage(QString::fromUtf8(error.what()));
    } catch (const BarcodeError& error) {
        details_->showMessage(QString::fromUtf8(error.what()));
    }
}

void OrderScreen::removeSelected() {
    int row = table_->currentRow();
    if (row < 0 || table_->item(row, 0) == nullptr) {
        setStatus("Select the item you want to remove.", true);
        return;
    }
    try {
        orders_.removeItem(table_->item(row, 0)->text().toStdString());
        refreshTable();
        recalculateTotals();
        setStatus("Item removed.", false);
    } catch (const PosError& error) {
        setStatus(QString::fromUtf8(error.what()), true);
    } catch (const BarcodeError& error) {
        setStatus(QString::fromUtf8(error.what()), true);
    }
}

void OrderScreen::incrementSelected() {
    std::string barcode = selectedBarcode();
    if (barcode.empty()) {
        setStatus("Select an item in the table to increase quantity.", true);
        return;
    }

    const auto& lines = orders_.draft().lines;
    for (const auto& line : lines) {
        if (line.barcode == barcode) {
            try {
                orders_.updateItemQuantity(barcode, line.quantity + 1.0);
                refreshTable();
                recalculateTotals();
                setStatus("Increased quantity.", false);
            } catch (const PosError& error) {
                setStatus(QString::fromUtf8(error.what()), true);
            }
            return;
        }
    }
    setStatus("Product not found in current order.", true);
}

void OrderScreen::decrementSelected() {
    std::string barcode = selectedBarcode();
    if (barcode.empty()) {
        setStatus("Select an item in the table to decrease quantity.", true);
        return;
    }

    const auto& lines = orders_.draft().lines;
    for (const auto& line : lines) {
        if (line.barcode == barcode) {
            if (line.quantity <= 1.0) {
                removeSelected();
                return;
            }
            try {
                orders_.updateItemQuantity(barcode, line.quantity - 1.0);
                refreshTable();
                recalculateTotals();
                setStatus("Decreased quantity.", false);
            } catch (const PosError& error) {
                setStatus(QString::fromUtf8(error.what()), true);
            }
            return;
        }
    }
    setStatus("Product not found in current order.", true);
}

void OrderScreen::editQuantitySelected() {
    std::string barcode = selectedBarcode();
    if (barcode.empty()) {
        setStatus("Select an item to edit quantity.", true);
        return;
    }

    double currentQty = 1.0;
    for (const auto& line : orders_.draft().lines) {
        if (line.barcode == barcode) {
            currentQty = line.quantity;
            break;
        }
    }

    bool ok = false;
    double newQty = QInputDialog::getDouble(
        this, "Edit Quantity", "New quantity for product " + QString::fromStdString(barcode) + ":",
        currentQty, 0.01, 9999.0, 2, &ok);

    if (ok && newQty > 0.0) {
        try {
            orders_.updateItemQuantity(barcode, newQty);
            refreshTable();
            recalculateTotals();
            setStatus("Quantity updated to " + QString::number(newQty), false);
        } catch (const PosError& error) {
            setStatus(QString::fromUtf8(error.what()), true);
        }
    }
}

void OrderScreen::newOrder() {
    orders_.start();
    clearForm();
    refreshTable();
    updateHoldButtonState();
    recalculateTotals();
    setStatus("Started new order.", false);
}

void OrderScreen::holdOrder() {
    try {
        orders_.hold();
        clearForm();
        refreshTable();
        updateHoldButtonState();
        recalculateTotals();
        setStatus("Active order put on hold. Started new order.", false);
    } catch (const PosError& error) {
        setStatus(QString::fromUtf8(error.what()), true);
    }
}

void OrderScreen::viewHeldOrders() {
    auto held = orders_.heldOrders();
    if (held.empty()) {
        setStatus("No orders are currently on hold.", true);
        return;
    }
    HeldOrdersDialog dialog(held, this);
    if (dialog.exec() == QDialog::Accepted) {
        std::string selectedId = dialog.selectedOrderId();
        if (!selectedId.empty()) {
            resumeSpecificOrder(selectedId);
        }
    }
}

void OrderScreen::resumeSpecificOrder(const std::string& orderId) {
    try {
        orders_.resume(orderId);
        const auto& d = orders_.draft();
        customerEdit_->setText(QString::fromStdString(d.customerName));
        contactEdit_->setText(QString::fromStdString(d.contact));
        paymentCombo_->setCurrentText(QString::fromStdString(pos::toString(d.mode)));
        merchantEdit_->setText(QString::fromStdString(d.merchant));

        refreshTable();
        updateHoldButtonState();
        recalculateTotals();
        setStatus("Resumed order " + QString::fromStdString(d.id), false);
    } catch (const PosError& error) {
        setStatus(QString::fromUtf8(error.what()), true);
    }
}

void OrderScreen::resumeOrder() {
    viewHeldOrders();
}

void OrderScreen::openEditOrder() {
    bool ok = false;
    QString orderId = QInputDialog::getText(
        this, "Open / Edit Order", "Enter Order ID to edit:",
        QLineEdit::Normal, "", &ok);

    if (!ok || orderId.trimmed().isEmpty()) return;

    std::string id = orderId.trimmed().toStdString();

    try {
        // Attempt load without manager approval first
        orders_.loadOrderForEdit(id, false);
    } catch (const PosError& e) {
        std::string err = e.what();
        if (err.find("requires manager approval") != std::string::npos) {
            // Prompt manager approval dialog
            bool approved = ManagerApprovalDialog::requestApproval(
                this, managerPin_, "Editing paid order '" + QString::fromStdString(id) + "' requires Manager PIN:");
            if (!approved) {
                setStatus("Edit cancelled: Manager authorization required.", true);
                return;
            }
            try {
                orders_.loadOrderForEdit(id, true);
            } catch (const PosError& e2) {
                QMessageBox::critical(this, "Cannot Edit Order", QString::fromStdString(e2.what()));
                return;
            }
        } else {
            QMessageBox::critical(this, "Cannot Edit Order", QString::fromStdString(err));
            return;
        }
    }

    const auto& d = orders_.draft();
    customerEdit_->setText(QString::fromStdString(d.customerName));
    contactEdit_->setText(QString::fromStdString(d.contact));
    paymentCombo_->setCurrentText(QString::fromStdString(pos::toString(d.mode)));
    merchantEdit_->setText(QString::fromStdString(d.merchant));

    refreshTable();
    updateHoldButtonState();
    recalculateTotals();
    setStatus("Editing order " + QString::fromStdString(id) + ". Adjust items and click Generate Bill.", false);
}

void OrderScreen::onPaymentInputsChanged() {
    recalculateTotals();
}

void OrderScreen::recalculateTotals() {
    if (!orders_.hasActiveDraft() || orders_.draft().lines.empty()) {
        totalTaxLabel_->setText("0.00 PKR");
        totalDiscountLabel_->setText("0.00 PKR");
        payableAmountLabel_->setText("0.00 PKR");
        balanceReturnedLabel_->setText("0.00 PKR");
        return;
    }

    try {
        double specialPercent = specialDiscountSpin_->value();
        Bill bill = checkout_.previewBill(orders_.draft(), specialPercent, Money());

        totalTaxLabel_->setText(QString::fromStdString(bill.totalTax.toString()) + " PKR");

        Money allDiscounts = bill.totalDiscount + bill.seasonalDiscount + bill.specialDiscount;
        totalDiscountLabel_->setText("-" + QString::fromStdString(allDiscounts.toString()) + " PKR");
        payableAmountLabel_->setText(QString::fromStdString(bill.totalPayable.toString()) + " PKR");

        double recVal = amountReceivedSpin_->value();
        if (recVal <= 0.0) {
            amountReceivedSpin_->setValue(bill.totalPayable.paisa() / 100.0);
            recVal = bill.totalPayable.paisa() / 100.0;
        }

        Money received = Money::fromDouble(recVal);
        Money balance = (received.paisa() >= bill.totalPayable.paisa()) ? (received - bill.totalPayable) : Money();
        balanceReturnedLabel_->setText(QString::fromStdString(balance.toString()) + " PKR");
    } catch (const std::exception&) {
        // Fallback
    }
}

void OrderScreen::generateBill() {
    try {
        std::string customer = customerEdit_->text().trimmed().toStdString();
        if (customer.empty()) customer = "Walk-in";
        std::string contact = contactEdit_->text().trimmed().toStdString();
        if (contact.empty()) contact = "-";
        orders_.setCustomer(customer, contact);

        PaymentMode mode = pos::paymentModeFromString(paymentCombo_->currentText().toStdString());
        std::string merchant = merchantEdit_->text().trimmed().toStdString();
        orders_.setPayment(mode, merchant);

        double specialPercent = specialDiscountSpin_->value();
        Money received = Money::fromDouble(amountReceivedSpin_->value());

        Order finalizedOrder = orders_.draft();
        Bill bill = checkout_.checkout(specialPercent, received);

        BillDialog dialog(bill, finalizedOrder, receiptsDir_, shopName_, this);
        dialog.exec();

        newOrder();
    } catch (const PosError& error) {
        setStatus(QString::fromUtf8(error.what()), true);
    } catch (const BarcodeError& error) {
        setStatus(QString::fromUtf8(error.what()), true);
    }
}

void OrderScreen::updateHoldButtonState() {
    auto held = orders_.heldOrders();
    viewHeldButton_->setEnabled(!held.empty());
    viewHeldButton_->setText(QString("📋 View Held Orders [%1]").arg(held.size()));
}

void OrderScreen::refreshTable() {
    table_->setRowCount(0);
    if (!orders_.hasActiveDraft()) return;

    const auto& lines = orders_.draft().lines;
    for (int row = 0; row < static_cast<int>(lines.size()); ++row) {
        const auto& line = lines[row];
        table_->insertRow(row);
        table_->setItem(row, 0, makeItem(QString::fromStdString(line.barcode)));
        table_->setItem(row, 1, makeItem(QString::fromStdString(line.acronym)));
        table_->setItem(row, 2, makeItem(QString::fromStdString(line.unitPrice.toString())));
        table_->setItem(row, 3, makeItem(QString::number(line.quantity)));
        table_->setItem(row, 4,
                        makeItem(QString::fromStdString((line.unitPrice * line.quantity).toString())));
    }
}

void OrderScreen::clearForm() {
    customerEdit_->clear();
    contactEdit_->clear();
    merchantEdit_->clear();
    paymentCombo_->setCurrentIndex(0);
    specialDiscountSpin_->setValue(0.0);
    amountReceivedSpin_->setValue(0.0);
    totalTaxLabel_->setText("0.00 PKR");
    totalDiscountLabel_->setText("0.00 PKR");
    payableAmountLabel_->setText("0.00 PKR");
    balanceReturnedLabel_->setText("0.00 PKR");
    status_->setText("Scan a product or click New Order.");
}

void OrderScreen::setStatus(const QString& text, bool isError) {
    status_->setText(text);
    status_->setStyleSheet(isError ? "color: #dc2626; font-size: 12px; font-weight: bold;"
                                   : "color: #16a34a; font-size: 12px; font-weight: bold;");
}
