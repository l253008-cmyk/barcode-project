#include "pos/gui/OrderScreen.hpp"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
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

    QPushButton* descriptionButton = new QPushButton("Description", this);
    QPushButton* removeButton = new QPushButton("Remove Item", this);
    QPushButton* billButton = new QPushButton("Generate Bill", this);

    connect(descriptionButton, &QPushButton::clicked, this, &OrderScreen::describeSelected);
    connect(removeButton, &QPushButton::clicked, this, &OrderScreen::removeSelected);
    connect(billButton, &QPushButton::clicked, this, &OrderScreen::generateBill);

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

    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addWidget(descriptionButton);
    buttons->addWidget(removeButton);
    buttons->addWidget(billButton);

    QVBoxLayout* right = new QVBoxLayout();
    right->addWidget(table_, 1);
    right->addLayout(customerForm);
    right->addLayout(buttons);
    right->addWidget(status_);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addLayout(left, 1);
    layout->addLayout(right, 2);

    orders_.start();
    refreshTable();
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
