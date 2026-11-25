// HeldOrdersDialog.cpp                             Owner: Person 1
// Dialog showing a preview of all parked/held orders with individual Resume buttons.

#include "pos/gui/HeldOrdersDialog.hpp"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

HeldOrdersDialog::HeldOrdersDialog(const std::vector<Order>& heldOrders, QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("?? Held / Parked Orders Preview");
    resize(760, 420);
    setStyleSheet("QDialog { background-color: #F8FAFC; color: #0F172A; }");
    setupUi(heldOrders);
}

void HeldOrdersDialog::setupUi(const std::vector<Order>& heldOrders) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    auto* titleLabel = new QLabel("?? <b>Active Held Orders</b>", this);
    titleLabel->setStyleSheet("font-size: 16px; color: #0F172A;");
    mainLayout->addWidget(titleLabel);

    auto* subtitle = new QLabel("Below is the preview of all transactions parked on hold. Click <b>Resume</b> to load an order back into the cashier terminal.", this);
    subtitle->setStyleSheet("color: #475569; font-size: 12px;");
    subtitle->setWordWrap(true);
    mainLayout->addWidget(subtitle);

    if (heldOrders.empty()) {
        auto* emptyLabel = new QLabel("No orders are currently on hold.", this);
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("font-size: 14px; color: #64748B; padding: 40px; font-style: italic;");
        mainLayout->addWidget(emptyLabel);
    }
    else {
        table_ = new QTableWidget(this);
        table_->setColumnCount(7);
        table_->setHorizontalHeaderLabels({ "Order ID", "Customer", "Contact", "Date & Time", "Items", "Total", "Action" });
        table_->setRowCount(static_cast<int>(heldOrders.size()));
        table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table_->setSelectionBehavior(QAbstractItemView::SelectRows);
        table_->verticalHeader()->setVisible(false);

        table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        table_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        table_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
        table_->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
        table_->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);

        table_->setStyleSheet(
            "QTableWidget { background-color: #FFFFFF; color: #0F172A; border: 1.5px solid #CBD5E1; border-radius: 6px; font-size: 12px; }"
            "QHeaderView::section { background-color: #F1F5F9; color: #0F172A; font-weight: bold; padding: 6px; border: none; border-bottom: 2px solid #CBD5E1; }"
        );

        for (int i = 0; i < static_cast<int>(heldOrders.size()); ++i) {
            const auto& ord = heldOrders[i];

            auto* idItem = new QTableWidgetItem(QString::fromStdString(ord.id));
            idItem->setTextAlignment(Qt::AlignCenter);
            table_->setItem(i, 0, idItem);

            auto* custItem = new QTableWidgetItem(QString::fromStdString(ord.customerName.empty() ? "Walk-in" : ord.customerName));
            table_->setItem(i, 1, custItem);

            auto* contactItem = new QTableWidgetItem(QString::fromStdString(ord.contact.empty() ? "-" : ord.contact));
            contactItem->setTextAlignment(Qt::AlignCenter);
            table_->setItem(i, 2, contactItem);

            QString dateTime = QString::fromStdString(ord.date + " " + ord.time);
            auto* dtItem = new QTableWidgetItem(dateTime);
            dtItem->setTextAlignment(Qt::AlignCenter);
            table_->setItem(i, 3, dtItem);

            auto* itemsItem = new QTableWidgetItem(QString::number(ord.lines.size()) + " lines");
            itemsItem->setTextAlignment(Qt::AlignCenter);
            table_->setItem(i, 4, itemsItem);

            auto* totalItem = new QTableWidgetItem(QString::fromStdString(ord.payment.toString()) + " PKR");
            totalItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            totalItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
            table_->setItem(i, 5, totalItem);

            // Small Resume button
            auto* resumeBtn = new QPushButton("?? Resume", table_);
            resumeBtn->setStyleSheet(
                "QPushButton { background-color: #4F46E5; color: #FFFFFF; font-weight: bold; padding: 4px 10px; border-radius: 4px; border: none; font-size: 11px; } "
                "QPushButton:hover { background-color: #4338CA; }"
            );
            QString currentId = QString::fromStdString(ord.id);
            connect(resumeBtn, &QPushButton::clicked, this, [this, currentId]() {
                onResumeClicked(currentId);
                });

            table_->setCellWidget(i, 6, resumeBtn);
        }

        mainLayout->addWidget(table_);
    }

    // Bottom close button
    auto* bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();
    auto* closeBtn = new QPushButton("Close Preview", this);
    closeBtn->setStyleSheet("QPushButton { padding: 6px 16px; background-color: #E2E8F0; color: #1E293B; font-weight: 600; border-radius: 4px; border: 1px solid #CBD5E1; } QPushButton:hover { background-color: #CBD5E1; }");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    bottomLayout->addWidget(closeBtn);
    mainLayout->addLayout(bottomLayout);
}

void HeldOrdersDialog::onResumeClicked(const QString& orderId) {
    selectedOrderId_ = orderId.toStdString();
    accept();
}