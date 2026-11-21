#include "pos/gui/OrderScreen.hpp"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include "barcode/error.hpp"
#include "pos/common/PosError.hpp"
#include "pos/domain/Bill.hpp"
#include "pos/domain/Order.hpp"
#include "pos/gui/BillDialog.hpp"
#include "pos/gui/CameraView.hpp"
#include "pos/gui/ProductDetailsWidget.hpp"
#include "pos/services/CheckoutService.hpp"
#include "pos/services/OrderService.hpp"
#include "pos/services/ProductService.hpp"

static QTableWidgetItem* makeItem(const QString& text) {
    QTableWidgetItem* item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
}

OrderScreen::OrderScreen(OrderService& orders, ProductService& products, CheckoutService& checkout,
                         QWidget* parent)
    : QWidget(parent), orders_(orders), products_(products), checkout_(checkout) {
    camera_ = new CameraView(this);
    details_ = new ProductDetailsWidget(this);

    quantity_ = new QDoubleSpinBox(this);
    quantity_->setRange(0.01, 9999.0);
    quantity_->setDecimals(2);
    quantity_->setValue(1.0);

    table_ = new QTableWidget(0, 5, this);
    table_->setHorizontalHeaderLabels(
        QStringList{"Barcode", "Product", "Unit price", "Quantity", "Amount"});
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);

    customerEdit_ = new QLineEdit(this);
    contactEdit_ = new QLineEdit(this);
    paymentCombo_ = new QComboBox(this);
    paymentCombo_->addItems(QStringList{"cash", "card", "wallet"});
    merchantEdit_ = new QLineEdit(this);

    status_ = new QLabel("Scan a product to add it to the order.", this);
    status_->setWordWrap(true);

    // Item manipulation buttons row
    QPushButton* incBtn = new QPushButton("➕ 1", this);
    QPushButton* decBtn = new QPushButton("➖ 1", this);
    QPushButton* editQtyBtn = new QPushButton("✏️ Edit Qty", this);
    QPushButton* descriptionButton = new QPushButton("ℹ️ Description", this);
    QPushButton* removeButton = new QPushButton("🗑️ Remove Item", this);

    incBtn->setToolTip("Increase quantity of selected item by 1");
    decBtn->setToolTip("Decrease quantity of selected item by 1");
    editQtyBtn->setToolTip("Enter exact quantity for selected item");

    incBtn->setStyleSheet(
        "QPushButton { background-color: #EFF6FF; color: #1D4ED8; font-weight: bold; font-size: 13px; "
        "border: 2px solid #3B82F6; border-radius: 6px; padding: 7px 14px; } "
        "QPushButton:hover { background-color: #DBEAFE; } "
        "QPushButton:pressed { background-color: #BFDBFE; }");

    decBtn->setStyleSheet(
        "QPushButton { background-color: #FEF2F2; color: #B91C1C; font-weight: bold; font-size: 13px; "
        "border: 2px solid #EF4444; border-radius: 6px; padding: 7px 14px; } "
        "QPushButton:hover { background-color: #FEE2E2; } "
        "QPushButton:pressed { background-color: #FECACA; }");

    editQtyBtn->setStyleSheet(
        "QPushButton { background-color: #F8FAFC; color: #0F172A; font-weight: bold; font-size: 13px; "
        "border: 2px solid #64748B; border-radius: 6px; padding: 7px 14px; } "
        "QPushButton:hover { background-color: #E2E8F0; }");

    descriptionButton->setStyleSheet(
        "QPushButton { background-color: #F1F5F9; color: #0F172A; font-weight: 600; font-size: 13px; "
        "border: 1.5px solid #94A3B8; border-radius: 6px; padding: 7px 12px; } "
        "QPushButton:hover { background-color: #E2E8F0; }");

    removeButton->setStyleSheet(
        "QPushButton { background-color: #DC2626; color: #FFFFFF; font-weight: bold; font-size: 13px; "
        "border: 1.5px solid #B91C1C; border-radius: 6px; padding: 7px 14px; } "
        "QPushButton:hover { background-color: #B91C1C; }");

    connect(incBtn, &QPushButton::clicked, this, &OrderScreen::incrementSelected);
    connect(decBtn, &QPushButton::clicked, this, &OrderScreen::decrementSelected);
    connect(editQtyBtn, &QPushButton::clicked, this, &OrderScreen::editQuantitySelected);
    connect(descriptionButton, &QPushButton::clicked, this, &OrderScreen::describeSelected);
    connect(removeButton, &QPushButton::clicked, this, &OrderScreen::removeSelected);

    QHBoxLayout* itemButtons = new QHBoxLayout();
    itemButtons->addWidget(incBtn);
    itemButtons->addWidget(decBtn);
    itemButtons->addWidget(editQtyBtn);
    itemButtons->addWidget(descriptionButton);
    itemButtons->addWidget(removeButton);

    // Order action buttons row
    holdButton_ = new QPushButton("⏸️ Hold Order", this);
    resumeButton_ = new QPushButton("▶️ Resume Held Order (0)", this);
    resumeButton_->setEnabled(false);
    QPushButton* billButton = new QPushButton("🧾 Generate Bill", this);

    holdButton_->setStyleSheet(
        "QPushButton { background-color: #D97706; color: #FFFFFF; font-weight: bold; font-size: 13px; "
        "padding: 8px 14px; border-radius: 6px; border: none; } "
        "QPushButton:hover { background-color: #B45309; }");

    resumeButton_->setStyleSheet(
        "QPushButton { background-color: #4F46E5; color: #FFFFFF; font-weight: bold; font-size: 13px; "
        "padding: 8px 14px; border-radius: 6px; border: none; } "
        "QPushButton:hover { background-color: #4338CA; } "
        "QPushButton:disabled { background-color: #E2E8F0; color: #94A3B8; border: 1px solid #CBD5E1; }");

    billButton->setStyleSheet(
        "QPushButton { background-color: #10B981; color: white; font-weight: bold; font-size: 13px; "
        "padding: 8px 16px; border-radius: 6px; border: none; } "
        "QPushButton:hover { background-color: #059669; }");

    connect(holdButton_, &QPushButton::clicked, this, &OrderScreen::holdOrder);
    connect(resumeButton_, &QPushButton::clicked, this, &OrderScreen::resumeOrder);
    connect(billButton, &QPushButton::clicked, this, &OrderScreen::generateBill);

    QHBoxLayout* orderButtons = new QHBoxLayout();
    orderButtons->addWidget(holdButton_);
    orderButtons->addWidget(resumeButton_);
    orderButtons->addWidget(billButton);

    QFormLayout* quantityForm = new QFormLayout();
    quantityForm->addRow("Quantity per scan:", quantity_);

    QVBoxLayout* left = new QVBoxLayout();
    left->addWidget(camera_, 1);
    left->addLayout(quantityForm);
    left->addWidget(details_, 1);

    QFormLayout* customerForm = new QFormLayout();
    customerForm->addRow("Customer name:", customerEdit_);
    customerForm->addRow("Contact:", contactEdit_);
    customerForm->addRow("Payment mode:", paymentCombo_);
    customerForm->addRow("Merchant:", merchantEdit_);

    QVBoxLayout* right = new QVBoxLayout();
    right->addWidget(table_, 1);
    right->addLayout(itemButtons);
    right->addLayout(customerForm);
    right->addLayout(orderButtons);
    right->addWidget(status_);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addLayout(left, 1);
    layout->addLayout(right, 2);

    orders_.start();
    refreshTable();
    updateHoldButtonState();
}

void OrderScreen::showFrame(const Image& frame) {
    camera_->showFrame(frame);
}

void OrderScreen::handleCode(const std::string& code) {
    lastCode_ = code;
    try {
        orders_.addItem(code, quantity_->value());
        refreshTable();
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
                setStatus("Updated quantity for " + QString::fromStdString(line.acronym) + " to " + QString::number(line.quantity + 1.0), false);
            } catch (const PosError& e) {
                setStatus(QString::fromUtf8(e.what()), true);
            }
            return;
        }
    }
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
            if (line.quantity > 1.0) {
                try {
                    orders_.updateItemQuantity(barcode, line.quantity - 1.0);
                    refreshTable();
                    setStatus("Updated quantity for " + QString::fromStdString(line.acronym) + " to " + QString::number(line.quantity - 1.0), false);
                } catch (const PosError& e) {
                    setStatus(QString::fromUtf8(e.what()), true);
                }
            } else {
                removeSelected();
            }
            return;
        }
    }
}

void OrderScreen::editQuantitySelected() {
    std::string barcode = selectedBarcode();
    if (barcode.empty()) {
        setStatus("Select an item in the table to edit quantity.", true);
        return;
    }

    double currentQty = 1.0;
    QString acronym = QString::fromStdString(barcode);
    for (const auto& line : orders_.draft().lines) {
        if (line.barcode == barcode) {
            currentQty = line.quantity;
            acronym = QString::fromStdString(line.acronym);
            break;
        }
    }

    bool ok = false;
    double newQty = QInputDialog::getDouble(this, "Edit Quantity",
        QString("Enter new quantity for %1:").arg(acronym),
        currentQty, 0.01, 99999.0, 2, &ok);

    if (ok && newQty > 0) {
        try {
            orders_.updateItemQuantity(barcode, newQty);
            refreshTable();
            setStatus(QString("Updated quantity for %1 to %2").arg(acronym).arg(newQty), false);
        } catch (const PosError& e) {
            setStatus(QString::fromUtf8(e.what()), true);
        }
    }
}

void OrderScreen::holdOrder() {
    if (orders_.draft().lines.empty()) {
        setStatus("Cannot hold an empty order.", true);
        return;
    }

    try {
        orders_.holdCurrentOrder();
        clearForm();
        refreshTable();
        updateHoldButtonState();
        setStatus("Order put on hold. Ready for the next customer.", false);
    } catch (const PosError& e) {
        setStatus(QString::fromUtf8(e.what()), true);
    }
}

void OrderScreen::resumeOrder() {
    if (!orders_.hasHeldOrders()) {
        setStatus("No held orders to resume.", true);
        return;
    }

    if (!orders_.draft().lines.empty()) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Active Order in Progress",
            "An order is currently in progress. Do you want to hold the current order before resuming the previous one?",
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if (reply == QMessageBox::Yes) {
            try {
                orders_.holdCurrentOrder();
            } catch (const PosError& e) {
                setStatus(QString::fromUtf8(e.what()), true);
                return;
            }
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }

    try {
        orders_.resumeHeldOrder(0);
        customerEdit_->setText(QString::fromStdString(orders_.draft().customerName));
        contactEdit_->setText(QString::fromStdString(orders_.draft().contact == "-" ? "" : orders_.draft().contact));
        refreshTable();
        updateHoldButtonState();
        setStatus("Resumed held order.", false);
    } catch (const PosError& e) {
        setStatus(QString::fromUtf8(e.what()), true);
    }
}

void OrderScreen::updateHoldButtonState() {
    if (resumeButton_) {
        bool hasHeld = orders_.hasHeldOrders();
        size_t count = orders_.getHeldOrders().size();
        resumeButton_->setEnabled(hasHeld);
        resumeButton_->setText(QString("Resume Held Order (%1)").arg(count));
    }
}


void OrderScreen::generateBill() {
    if (orders_.draft().lines.empty()) {
        setStatus("Scan at least one product before generating a bill.", true);
        return;
    }

    QString customer = customerEdit_->text().trimmed();
    if (customer.isEmpty()) {
        setStatus("Enter the customer name.", true);
        return;
    }

    PaymentMode mode = PaymentMode::Cash;
    if (paymentCombo_->currentIndex() == 1) {
        mode = PaymentMode::Card;
    } else if (paymentCombo_->currentIndex() == 2) {
        mode = PaymentMode::Wallet;
    }

    try {
        orders_.setCustomer(customer.toStdString(), contactEdit_->text().trimmed().toStdString());
        orders_.setPayment(mode, merchantEdit_->text().trimmed().toStdString());

        Order order = orders_.draft();
        Bill bill = checkout_.checkout();

        BillDialog dialog(bill, order, this);
        dialog.exec();

        orders_.start();
        clearForm();
        refreshTable();
        updateHoldButtonState();
        setStatus("Bill generated. Ready for the next customer.", false);
    } catch (const PosError& error) {
        setStatus(QString::fromUtf8(error.what()), true);
    } catch (const BarcodeError& error) {
        setStatus(QString::fromUtf8(error.what()), true);
    }
}

void OrderScreen::refreshTable() {
    const auto& lines = orders_.draft().lines;
    table_->setRowCount(static_cast<int>(lines.size()));

    for (int row = 0; row < static_cast<int>(lines.size()); ++row) {
        const OrderLine& line = lines[row];
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
    quantity_->setValue(1.0);
    details_->clear();
    lastCode_.clear();
}

void OrderScreen::setStatus(const QString& text, bool isError) {
    status_->setStyleSheet(isError ? "color: #b00020;" : "color: #1b5e20;");
    status_->setText(text);
}
