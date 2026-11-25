// ManagerApprovalDialog.cpp                        Owner: Person 1
// Dialog prompting for Manager PIN approval to perform restricted operations.

#include "pos/gui/ManagerApprovalDialog.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>

ManagerApprovalDialog::ManagerApprovalDialog(const std::string& expectedPin,
    const QString& message,
    QWidget* parent)
    : QDialog(parent), expectedPin_(expectedPin) {
    setWindowTitle("Manager Authorization");
    setModal(true);
    setFixedSize(360, 200);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    auto* msgLabel = new QLabel(message, this);
    msgLabel->setWordWrap(true);
    msgLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #1e293b;");
    mainLayout->addWidget(msgLabel);

    pinEdit_ = new QLineEdit(this);
    pinEdit_->setEchoMode(QLineEdit::Password);
    pinEdit_->setPlaceholderText("Enter 4-digit PIN");
    pinEdit_->setStyleSheet(
        "QLineEdit {"
        "  padding: 8px 12px;"
        "  font-size: 14px;"
        "  border: 1px solid #cbd5e1;"
        "  border-radius: 6px;"
        "  background: #ffffff;"
        "  color: #0f172a;"
        "}"
        "QLineEdit:focus {"
        "  border: 2px solid #2563eb;"
        "}"
    );
    mainLayout->addWidget(pinEdit_);

    errorLabel_ = new QLabel(this);
    errorLabel_->setStyleSheet("color: #dc2626; font-size: 12px; font-weight: 500;");
    errorLabel_->setVisible(false);
    mainLayout->addWidget(errorLabel_);

    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    auto* cancelBtn = new QPushButton("Cancel", this);
    cancelBtn->setStyleSheet(
        "QPushButton {"
        "  padding: 8px 16px;"
        "  border: 1px solid #cbd5e1;"
        "  border-radius: 6px;"
        "  background: #f8fafc;"
        "  color: #334155;"
        "  font-weight: 500;"
        "}"
        "QPushButton:hover { background: #e2e8f0; }"
    );
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(cancelBtn);

    auto* okBtn = new QPushButton("Approve", this);
    okBtn->setStyleSheet(
        "QPushButton {"
        "  padding: 8px 18px;"
        "  border: none;"
        "  border-radius: 6px;"
        "  background: #2563eb;"
        "  color: #ffffff;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background: #1d4ed8; }"
    );
    connect(okBtn, &QPushButton::clicked, this, &ManagerApprovalDialog::onVerify);
    connect(pinEdit_, &QLineEdit::returnPressed, this, &ManagerApprovalDialog::onVerify);
    btnLayout->addWidget(okBtn);

    mainLayout->addLayout(btnLayout);
    pinEdit_->setFocus();
}

void ManagerApprovalDialog::onVerify() {
    if (pinEdit_->text().toStdString() == expectedPin_) {
        accept();
    }
    else {
        errorLabel_->setText("Incorrect PIN. Please try again.");
        errorLabel_->setVisible(true);
        pinEdit_->selectAll();
        pinEdit_->setFocus();
    }
}

bool ManagerApprovalDialog::requestApproval(QWidget* parent,
    const std::string& expectedPin,
    const QString& message) {
    ManagerApprovalDialog dlg(expectedPin, message, parent);
    return dlg.exec() == QDialog::Accepted;
}