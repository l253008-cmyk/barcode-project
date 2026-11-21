#include "pos/gui/BillDialog.hpp"

#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QTableWidget>
#include <QVBoxLayout>

static QString paymentModeText(PaymentMode mode) {
    if (mode == PaymentMode::Card) {
        return "card";
    }
    if (mode == PaymentMode::Wallet) {
        return "wallet";
    }
    return "cash";
}

static QTableWidgetItem* makeItem(const QString& text) {
    QTableWidgetItem* item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
}

BillDialog::BillDialog(const Bill& bill, const Order& order, QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Bill");
    resize(640, 520);

    QString contact = QString::fromStdString(order.contact);
    QString merchant = QString::fromStdString(order.merchant);
    if (contact.isEmpty()) {
        contact = "-";
    }
    if (merchant.isEmpty()) {
        merchant = "-";
    }

    QFormLayout* info = new QFormLayout();
    info->addRow("Bill id:", new QLabel(QString::fromStdString(bill.id), this));
    info->addRow("Order id:", new QLabel(QString::fromStdString(bill.orderId), this));
    info->addRow("Customer:", new QLabel(QString::fromStdString(bill.customerName), this));
    info->addRow("Contact:", new QLabel(contact, this));
    info->addRow("Payment mode:", new QLabel(paymentModeText(order.mode), this));
    info->addRow("Merchant:", new QLabel(merchant, this));

    QTableWidget* table = new QTableWidget(static_cast<int>(bill.lines.size()), 5, this);
    table->setHorizontalHeaderLabels(
        QStringList{"Product", "Unit price", "Discount per unit", "Quantity", "Amount"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionMode(QAbstractItemView::NoSelection);

    for (int row = 0; row < static_cast<int>(bill.lines.size()); ++row) {
        const BillLine& line = bill.lines[row];
        table->setItem(row, 0, makeItem(QString::fromStdString(line.acronym)));
        table->setItem(row, 1, makeItem(QString::fromStdString(line.unitPrice.toString())));
        table->setItem(row, 2, makeItem(QString::fromStdString(line.discount.toString())));
        table->setItem(row, 3, makeItem(QString::number(line.quantity)));
        table->setItem(row, 4,
                       makeItem(QString::fromStdString((line.unitPrice * line.quantity).toString())));
    }

    QFormLayout* totals = new QFormLayout();
    totals->addRow("Total without tax:",
                   new QLabel(QString::fromStdString(bill.totalWithoutTax.toString()), this));
    totals->addRow("Total tax:", new QLabel(QString::fromStdString(bill.totalTax.toString()), this));
    totals->addRow("Total discount:",
                   new QLabel(QString::fromStdString(bill.totalDiscount.toString()), this));

    QLabel* payable = new QLabel(QString::fromStdString(bill.totalPayable.toString()), this);
    QFont font = payable->font();
    font.setBold(true);
    font.setPointSize(font.pointSize() + 3);
    payable->setFont(font);
    totals->addRow("Total payable:", payable);

    QPushButton* closeButton = new QPushButton("Close", this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(info);
    layout->addWidget(table);
    layout->addLayout(totals);
    layout->addWidget(closeButton);
}
