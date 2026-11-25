// BillDialog.cpp                                      Owner: Person 1
// Dialog displaying completed bill summary and native PDF receipt printing.

#include "pos/gui/BillDialog.hpp"
#include "pos/gui/PdfReceiptPrinter.hpp"
#include "pos/common/PosError.hpp"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QTableWidget>
#include <QVBoxLayout>

static QString paymentModeText(PaymentMode mode) {
    if (mode == PaymentMode::Card) return "Card";
    if (mode == PaymentMode::Wallet) return "Wallet";
    return "Cash";
}

static QTableWidgetItem* makeItem(const QString& text) {
    auto* item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
}

BillDialog::BillDialog(const Bill& bill, const Order& order,
    const std::string& receiptsDir,
    const std::string& shopName,
    QWidget* parent)
    : QDialog(parent), bill_(bill), receiptsDir_(receiptsDir), shopName_(shopName) {
    setWindowTitle("Bill & Receipt — " + QString::fromStdString(bill.id));
    resize(640, 580);

    QString contact = QString::fromStdString(order.contact);
    QString merchant = QString::fromStdString(order.merchant);
    if (contact.isEmpty()) contact = "-";
    if (merchant.isEmpty()) merchant = "-";

    auto* info = new QFormLayout();
    info->addRow("Bill ID:", new QLabel(QString::fromStdString(bill.id), this));
    info->addRow("Order ID:", new QLabel(QString::fromStdString(bill.orderId), this));
    info->addRow("Customer:", new QLabel(QString::fromStdString(bill.customerName), this));
    info->addRow("Contact:", new QLabel(contact, this));
    info->addRow("Payment Mode:", new QLabel(paymentModeText(order.mode), this));
    info->addRow("Date & Time:", new QLabel(QString::fromStdString(bill.date + " " + bill.time), this));

    auto* table = new QTableWidget(static_cast<int>(bill.lines.size()), 5, this);
    table->setHorizontalHeaderLabels(
        QStringList{ "Product", "Unit Price", "Discount/Unit", "Quantity", "Line Total" });
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setStyleSheet(
        "QTableWidget { background-color: #ffffff; color: #0f172a; border: 1px solid #e2e8f0; border-radius: 6px; }"
        "QHeaderView::section { background-color: #f1f5f9; color: #334155; font-weight: bold; padding: 6px; border: none; }"
    );

    for (int row = 0; row < static_cast<int>(bill.lines.size()); ++row) {
        const BillLine& line = bill.lines[row];
        table->setItem(row, 0, makeItem(QString::fromStdString(line.acronym)));
        table->setItem(row, 1, makeItem(QString::fromStdString(line.unitPrice.toString())));
        table->setItem(row, 2, makeItem(QString::fromStdString(line.discount.toString())));
        table->setItem(row, 3, makeItem(QString::number(line.quantity)));
        table->setItem(row, 4,
            makeItem(QString::fromStdString(((line.unitPrice * line.quantity) - line.discount).toString())));
    }

    auto* totals = new QFormLayout();
    totals->addRow("Total without tax:",
        new QLabel(QString::fromStdString(bill.totalWithoutTax.toString()) + " PKR", this));
    totals->addRow("Total tax:",
        new QLabel(QString::fromStdString(bill.totalTax.toString()) + " PKR", this));
    totals->addRow("Product discount:",
        new QLabel("-" + QString::fromStdString(bill.totalDiscount.toString()) + " PKR", this));

    if (bill.seasonalDiscount.paisa() > 0) {
        totals->addRow("Seasonal discount:",
            new QLabel("-" + QString::fromStdString(bill.seasonalDiscount.toString()) + " PKR", this));
    }
    if (bill.specialDiscount.paisa() > 0) {
        totals->addRow("Special discount (<= 8%):",
            new QLabel("-" + QString::fromStdString(bill.specialDiscount.toString()) + " PKR", this));
    }

    auto* payable = new QLabel(QString::fromStdString(bill.totalPayable.toString()) + " PKR", this);
    QFont font = payable->font();
    font.setBold(true);
    font.setPointSize(font.pointSize() + 3);
    payable->setFont(font);
    payable->setStyleSheet("color: #2563eb;");
    totals->addRow("Total Payable:", payable);

    totals->addRow("Amount Received:",
        new QLabel(QString::fromStdString(bill.amountReceived.toString()) + " PKR", this));
    totals->addRow("Balance Returned:",
        new QLabel(QString::fromStdString(bill.balanceReturned.toString()) + " PKR", this));

    printStatusLabel_ = new QLabel(this);
    printStatusLabel_->setStyleSheet("font-size: 12px; font-weight: bold;");
    printStatusLabel_->setVisible(false);

    // Buttons
    auto* btnLayout = new QHBoxLayout();

    auto* printBtn = new QPushButton("??? Print PDF Receipt", this);
    printBtn->setStyleSheet("QPushButton { padding: 8px 18px; background: #2563eb; color: #ffffff; font-weight: bold; border-radius: 6px; border: none; } QPushButton:hover { background: #1d4ed8; }");
    connect(printBtn, &QPushButton::clicked, this, &BillDialog::onPrintPdf);
    btnLayout->addWidget(printBtn);

    btnLayout->addStretch();

    auto* closeButton = new QPushButton("Close", this);
    closeButton->setStyleSheet("QPushButton { padding: 8px 18px; background: #64748b; color: #ffffff; font-weight: bold; border-radius: 6px; border: none; } QPushButton:hover { background: #475569; }");
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(closeButton);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(info);
    layout->addWidget(table);
    layout->addLayout(totals);
    layout->addWidget(printStatusLabel_);
    layout->addLayout(btnLayout);
}

void BillDialog::onPrintPdf() {
    try {
        std::string pdfPath = PdfReceiptPrinter::printReceipt(bill_, receiptsDir_, shopName_);
        printStatusLabel_->setText("? Receipt saved to: " + QString::fromStdString(pdfPath));
        printStatusLabel_->setStyleSheet("color: #16a34a; font-size: 12px; font-weight: bold;");
        printStatusLabel_->setVisible(true);
    }
    catch (const PosError& e) {
        printStatusLabel_->setText("? Failed to print PDF: " + QString::fromStdString(e.what()));
        printStatusLabel_->setStyleSheet("color: #dc2626; font-size: 12px; font-weight: bold;");
        printStatusLabel_->setVisible(true);
    }
}