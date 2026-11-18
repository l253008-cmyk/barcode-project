#pragma once

#include <QDialog>

#include "pos/domain/Bill.hpp"
#include "pos/domain/Order.hpp"

class BillDialog : public QDialog {
    Q_OBJECT

public:
    BillDialog(const Bill& bill, const Order& order, QWidget* parent = nullptr);
};
