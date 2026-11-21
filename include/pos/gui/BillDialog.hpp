// BillDialog.hpp                                      Owner: Person 1
// Dialog displaying completed bill summary and native PDF receipt printing.

#ifndef POS_BILL_DIALOG_HPP
#define POS_BILL_DIALOG_HPP

#include <QDialog>
#include <QLabel>
#include <string>

#include "pos/domain/Bill.hpp"
#include "pos/domain/Order.hpp"

class BillDialog : public QDialog {
    Q_OBJECT

public:
    BillDialog(const Bill& bill, const Order& order,
               const std::string& receiptsDir = "data/receipts",
               const std::string& shopName = "Store POS System",
               QWidget* parent = nullptr);

private slots:
    void onPrintPdf();

private:
    Bill bill_;
    std::string receiptsDir_;
    std::string shopName_;
    QLabel* printStatusLabel_ = nullptr;
};

#endif // POS_BILL_DIALOG_HPP
