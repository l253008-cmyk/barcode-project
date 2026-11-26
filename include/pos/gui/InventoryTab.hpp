// InventoryTab.hpp                                 Owner: Person 5
// Manager UI tab for direct inventory inspection and stock adjustment.

#ifndef POS_INVENTORY_TAB_HPP
#define POS_INVENTORY_TAB_HPP

#include <QWidget>
#include <QLineEdit>
#include <QTableWidget>
#include <QLabel>
#include <vector>

#include "pos/services/ProductService.hpp"

class InventoryTab : public QWidget {
    Q_OBJECT

public:
    explicit InventoryTab(ProductService& productService, QWidget* parent = nullptr);

    void refresh();

signals:
    void inventoryChanged();

private slots:
    void onSearchFilterChanged(const QString& filter);
    void onUpdateStockRow(int row);

private:
    void setupUi();
    void populateTable();

    ProductService& productService_;
    QLineEdit* searchEdit_ = nullptr;
    QTableWidget* table_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    std::vector<Product> filteredProducts_;
};

#endif // POS_INVENTORY_TAB_HPP
