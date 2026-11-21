#pragma once

#include <QDialog>

enum class UserRole { Cashier, Manager };

class RoleDialog : public QDialog {
    Q_OBJECT

public:
    explicit RoleDialog(QWidget* parent = nullptr);
    UserRole role() const;

private:
    UserRole role_;
};
