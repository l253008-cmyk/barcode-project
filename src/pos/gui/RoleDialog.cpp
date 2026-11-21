#include "pos/gui/RoleDialog.hpp"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

RoleDialog::RoleDialog(QWidget* parent)
    : QDialog(parent), role_(UserRole::Cashier) {
    setWindowTitle("Store POS - Choose role");

    QLabel* title = new QLabel("Who is using the system?", this);
    QPushButton* cashierButton = new QPushButton("Cashier", this);
    QPushButton* managerButton = new QPushButton("Manager", this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(title);
    layout->addWidget(cashierButton);
    layout->addWidget(managerButton);

    connect(cashierButton, &QPushButton::clicked, this, [this]() {
        role_ = UserRole::Cashier;
        accept();
    });
    connect(managerButton, &QPushButton::clicked, this, [this]() {
        role_ = UserRole::Manager;
        accept();
    });
}

UserRole RoleDialog::role() const {
    return role_;
}
