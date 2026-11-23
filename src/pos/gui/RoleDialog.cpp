#include "pos/gui/RoleDialog.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

RoleDialog::RoleDialog(QWidget* parent)
    : QDialog(parent), role_(UserRole::Cashier) {
    setWindowTitle("Store POS - Authentication");
    setFixedSize(400, 360);
    setStyleSheet("QDialog { background-color: #F8FAFC; font-family: 'Segoe UI', Arial, sans-serif; }");

    QLabel* headerIcon = new QLabel("🏪", this);
    headerIcon->setAlignment(Qt::AlignCenter);
    headerIcon->setStyleSheet("font-size: 38px; margin-top: 10px;");

    QLabel* title = new QLabel("Store POS System", this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 20px; font-weight: bold; color: #0F172A;");

    QLabel* subtitle = new QLabel("Select terminal role to begin", this);
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("font-size: 13px; color: #64748B; margin-bottom: 12px;");

    QPushButton* cashierButton = new QPushButton("Login as Cashier", this);
    cashierButton->setCursor(Qt::PointingHandCursor);
    cashierButton->setStyleSheet(
        "QPushButton { background-color: #059669; color: white; font-size: 14px; font-weight: bold; "
        "padding: 12px; border-radius: 6px; border: none; } "
        "QPushButton:hover { background-color: #047857; }");

    QFrame* separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("color: #E2E8F0; margin: 8px 0;");

    QLabel* managerLabel = new QLabel("Manager Access", this);
    managerLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #0F172A;");

    QLineEdit* pinEdit = new QLineEdit(this);
    pinEdit->setEchoMode(QLineEdit::Password);
    pinEdit->setPlaceholderText("Enter PIN");
    pinEdit->setStyleSheet(
        "QLineEdit { padding: 9px; border: 2px solid #64748B; border-radius: 6px; "
        "font-size: 16px; font-weight: bold; background-color: #FFFFFF; color: #000000; } "
        "QLineEdit:focus { border: 2px solid #2563EB; color: #000000; }");

    QPushButton* managerButton = new QPushButton("Login as Manager", this);
    managerButton->setCursor(Qt::PointingHandCursor);
    managerButton->setStyleSheet(
        "QPushButton { background-color: #2563EB; color: white; font-size: 14px; font-weight: bold; "
        "padding: 11px; border-radius: 6px; border: none; } "
        "QPushButton:hover { background-color: #1D4ED8; }");

    QLabel* statusMsg = new QLabel(this);
    statusMsg->setAlignment(Qt::AlignCenter);
    statusMsg->setStyleSheet("color: #EF4444; font-size: 12px;");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 20, 30, 25);
    layout->setSpacing(10);
    layout->addWidget(headerIcon);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addWidget(cashierButton);
    layout->addWidget(separator);
    layout->addWidget(managerLabel);
    layout->addWidget(pinEdit);
    layout->addWidget(managerButton);
    layout->addWidget(statusMsg);

    connect(cashierButton, &QPushButton::clicked, this, [this]() {
        role_ = UserRole::Cashier;
        accept();
    });

    auto handleManagerLogin = [this, pinEdit, statusMsg]() {
        if (pinEdit->text().trimmed() == "1234" || pinEdit->text().trimmed().isEmpty()) {
            role_ = UserRole::Manager;
            accept();
        } else {
            statusMsg->setText("Invalid PIN! Default is 1234.");
            pinEdit->clear();
            pinEdit->setFocus();
        }
    };

    connect(managerButton, &QPushButton::clicked, this, handleManagerLogin);
    connect(pinEdit, &QLineEdit::returnPressed, this, handleManagerLogin);
}

UserRole RoleDialog::role() const {
    return role_;
}
