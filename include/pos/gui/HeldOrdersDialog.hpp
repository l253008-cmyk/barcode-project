// HeldOrdersDialog.hpp                               Owner: Person 1
// Dialog showing a preview of all parked/held orders with individual Resume buttons.

#ifndef POS_HELD_ORDERS_DIALOG_HPP
#define POS_HELD_ORDERS_DIALOG_HPP

#include <QDialog>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <string>
#include <vector>

#include "pos/domain/Order.hpp"

class HeldOrdersDialog : public QDialog {
    Q_OBJECT

public:
    explicit HeldOrdersDialog(const std::vector<Order>& heldOrders, QWidget* parent = nullptr);

    // Returns the selected order ID if resumed, or empty string if dismissed
    std::string selectedOrderId() const { return selectedOrderId_; }

private slots:
    void onResumeClicked(const QString& orderId);

private:
    void setupUi(const std::vector<Order>& heldOrders);

    std::string selectedOrderId_;
    QTableWidget* table_ = nullptr;
};

#endif // POS_HELD_ORDERS_DIALOG_HPP