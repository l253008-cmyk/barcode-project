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

    status_ = new QLabel("Scan a product or press F2 for new order.", this);
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

    // Payment Box
    auto* paymentBox = new QGroupBox("Payment & Settlement", this);
    paymentBox->setStyleSheet("QGroupBox { font-weight: bold; font-size: 13px; color: #1e293b; }");
    auto* paymentLayout = new QGridLayout(paymentBox);

    paymentLayout->addWidget(new QLabel("Special Disc (% max 8):", paymentBox), 0, 0);
    specialDiscountSpin_ = new QDoubleSpinBox(paymentBox);
    specialDiscountSpin_->setRange(0.0, 8.0);
    specialDiscountSpin_->setSingleStep(0.5);
    specialDiscountSpin_->setSuffix(" %");
    specialDiscountSpin_->setStyleSheet("QDoubleSpinBox { padding: 4px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; }");
    connect(specialDiscountSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &OrderScreen::onPaymentInputsChanged);
    paymentLayout->addWidget(specialDiscountSpin_, 0, 1);

    paymentLayout->addWidget(new QLabel("Payable Amount:", paymentBox), 0, 2);
    payableAmountLabel_ = new QLabel("0.00 PKR", paymentBox);
    payableAmountLabel_->setStyleSheet("font-size: 14px; font-weight: bold; color: #2563eb;");
    paymentLayout->addWidget(payableAmountLabel_, 0, 3);

    paymentLayout->addWidget(new QLabel("Amount Received:", paymentBox), 1, 0);
    amountReceivedSpin_ = new QDoubleSpinBox(paymentBox);
    amountReceivedSpin_->setRange(0.0, 9999999.0);
    amountReceivedSpin_->setDecimals(2);
    amountReceivedSpin_->setStyleSheet("QDoubleSpinBox { padding: 4px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; font-weight: bold; }");
    connect(amountReceivedSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &OrderScreen::onPaymentInputsChanged);
    paymentLayout->addWidget(amountReceivedSpin_, 1, 1);

    paymentLayout->addWidget(new QLabel("Change / Balance:", paymentBox), 1, 2);
    balanceReturnedLabel_ = new QLabel("0.00 PKR", paymentBox);
    balanceReturnedLabel_->setStyleSheet("font-size: 14px; font-weight: bold; color: #16a34a;");
    paymentLayout->addWidget(balanceReturnedLabel_, 1, 3);

    // Order action buttons row with hotkey labels
    newOrderButton_ = new QPushButton("🆕 New (F2)", this);
    holdButton_ = new QPushButton("⏸️ Hold (F4)", this);
    resumeButton_ = new QPushButton("▶️ Resume (F5)", this);
    editOrderButton_ = new QPushButton("📂 Edit Order (F6)", this);
    billButton_ = new QPushButton("🧾 Bill (F12)", this);

    newOrderButton_->setStyleSheet("QPushButton { background-color: #64748b; color: #ffffff; font-weight: bold; padding: 8px 12px; border-radius: 6px; border: none; } QPushButton:hover { background-color: #475569; }");
    holdButton_->setStyleSheet("QPushButton { background-color: #d97706; color: #ffffff; font-weight: bold; padding: 8px 12px; border-radius: 6px; border: none; } QPushButton:hover { background-color: #b45309; }");
    resumeButton_->setStyleSheet("QPushButton { background-color: #4f46e5; color: #ffffff; font-weight: bold; padding: 8px 12px; border-radius: 6px; border: none; } QPushButton:hover { background-color: #4338ca; } QPushButton:disabled { background-color: #e2e8f0; color: #94a3b8; }");
    editOrderButton_->setStyleSheet("QPushButton { background-color: #0284c7; color: #ffffff; font-weight: bold; padding: 8px 12px; border-radius: 6px; border: none; } QPushButton:hover { background-color: #0369a1; }");
    billButton_->setStyleSheet("QPushButton { background-color: #10b981; color: #ffffff; font-weight: bold; padding: 8px 16px; border-radius: 6px; border: none; } QPushButton:hover { background-color: #059669; }");

    connect(newOrderButton_, &QPushButton::clicked, this, &OrderScreen::newOrder);
    connect(holdButton_, &QPushButton::clicked, this, &OrderScreen::holdOrder);
    connect(resumeButton_, &QPushButton::clicked, this, &OrderScreen::resumeOrder);
    connect(editOrderButton_, &QPushButton::clicked, this, &OrderScreen::openEditOrder);
    connect(billButton_, &QPushButton::clicked, this, &OrderScreen::generateBill);

    auto* orderButtons = new QHBoxLayout();
    orderButtons->addWidget(newOrderButton_);
    orderButtons->addWidget(holdButton_);
    orderButtons->addWidget(resumeButton_);
    orderButtons->addWidget(editOrderButton_);
    orderButtons->addWidget(billButton_);

    auto* quantityForm = new QFormLayout();
    quantityForm->addRow("Quantity per scan:", quantity_);

    auto* left = new QVBoxLayout();
    left->addWidget(camera_, 1);
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

    setupShortcuts();

    orders_.start();
    refreshTable();
    updateHoldButtonState();
}

void OrderScreen::setupShortcuts() {
    auto* scF2 = new QShortcut(QKeySequence(Qt::Key_F2), this);
    connect(scF2, &QShortcut::activated, this, &OrderScreen::newOrder);

    auto* scF4 = new QShortcut(QKeySequence(Qt::Key_F4), this);
    connect(scF4, &QShortcut::activated, this, &OrderScreen::holdOrder);

    auto* scF5 = new QShortcut(QKeySequence(Qt::Key_F5), this);
    connect(scF5, &QShortcut::activated, this, &OrderScreen::resumeOrder);

    auto* scF6 = new QShortcut(QKeySequence(Qt::Key_F6), this);
    connect(scF6, &QShortcut::activated, this, &OrderScreen::openEditOrder);

    auto* scF12 = new QShortcut(QKeySequence(Qt::Key_F12), this);
    connect(scF12, &QShortcut::activated, this, &OrderScreen::generateBill);
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

void OrderScreen::resumeOrder() {
    try {
        orders_.resumeLatest();
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

void OrderScreen::openEditOrder() {
    bool ok = false;
    QString orderId = QInputDialog::getText(
        this, "Open / Edit Order (F6)", "Enter Order ID to edit:",
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
    setStatus("Editing order " + QString::fromStdString(id) + ". Adjust items and press F12 to save.", false);
}

void OrderScreen::onPaymentInputsChanged() {
    recalculateTotals();
}

void OrderScreen::recalculateTotals() {
    if (!orders_.hasActiveDraft()) return;

    Money subtotal;
    for (const auto& line : orders_.draft().lines) {
        Money lineAmount = line.unitPrice * line.quantity;
        Money lineTax = line.tax * line.quantity;
        Money lineDiscount = line.discount * line.quantity;
        subtotal += (lineAmount + lineTax - lineDiscount);
    }

    double specialPercent = specialDiscountSpin_->value();
    double specialAmt = static_cast<double>(subtotal.paisa()) * (specialPercent / 100.0);
    Money specialDiscount = Money::fromPaisa(std::llround(specialAmt));
    Money payable = subtotal - specialDiscount;

    payableAmountLabel_->setText(QString::fromStdString(payable.toString()) + " PKR");

    double recVal = amountReceivedSpin_->value();
    if (recVal <= 0.0) {
        amountReceivedSpin_->setValue(payable.paisa() / 100.0);
        recVal = payable.paisa() / 100.0;
    }

    Money received = Money::fromDouble(recVal);
    Money balance = (received.paisa() >= payable.paisa()) ? (received - payable) : Money();
    balanceReturnedLabel_->setText(QString::fromStdString(balance.toString()) + " PKR");
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
    resumeButton_->setEnabled(!held.empty());
    resumeButton_->setText(QString("▶️ Resume (F5) [%1]").arg(held.size()));
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
    status_->setText("Scan a product or press F2 for new order.");
}

void OrderScreen::setStatus(const QString& text, bool isError) {
    status_->setText(text);
    status_->setStyleSheet(isError ? "color: #dc2626; font-size: 12px; font-weight: bold;"
                                   : "color: #16a34a; font-size: 12px; font-weight: bold;");
}
