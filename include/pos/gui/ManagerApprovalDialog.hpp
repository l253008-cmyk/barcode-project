// ManagerApprovalDialog.hpp                        Owner: Person 1
// Dialog prompting for Manager PIN approval to perform restricted operations.

#ifndef POS_MANAGER_APPROVAL_DIALOG_HPP
#define POS_MANAGER_APPROVAL_DIALOG_HPP

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <string>

class ManagerApprovalDialog : public QDialog {
    Q_OBJECT

public:
    explicit ManagerApprovalDialog(const std::string& expectedPin = "1234",
        const QString& message = "Enter Manager PIN to approve this action:",
        QWidget* parent = nullptr);

    // Static convenience method. Returns true if authorized.
    static bool requestApproval(QWidget* parent,
        const std::string& expectedPin = "1234",
        const QString& message = "Enter Manager PIN to approve editing this paid order:");

private slots:
    void onVerify();

private:
    std::string expectedPin_;
    QLineEdit* pinEdit_ = nullptr;
    QLabel* errorLabel_ = nullptr;
};

#endif // POS_MANAGER_APPROVAL_DIALOG_HPP