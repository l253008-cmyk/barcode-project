// InventoryTab.cpp                                 Owner: Person 5
// Manager UI tab for direct inventory inspection and stock adjustment.

#include "pos/gui/InventoryTab.hpp"
#include "pos/common/PosError.hpp"

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QVBoxLayout>

InventoryTab::InventoryTab(ProductService& productService, QWidget* parent)
    : QWidget(parent), productService_(productService) {
    setupUi();
    refresh();
}

void InventoryTab::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(14, 14, 14, 14);

    auto* topLayout = new QHBoxLayout();
    searchEdit_ = new QLineEdit(this);
    searchEdit_->setPlaceholderText("🔍 Search products by barcode, acronym, or name...");
    searchEdit_->setStyleSheet("QLineEdit { padding: 8px 12px; border: 1px solid #cbd5e1; border-radius: 6px; font-size: 13px; background: #ffffff; color: #0f172a; }");
    connect(searchEdit_, &QLineEdit::textChanged, this, &InventoryTab::onSearchFilterChanged);
    topLayout->addWidget(searchEdit_);

    auto* refreshBtn = new QPushButton("Refresh Stock", this);
    refreshBtn->setStyleSheet("QPushButton { padding: 8px 16px; background: #64748b; color: #ffffff; font-weight: bold; border-radius: 6px; border: none; } QPushButton:hover { background: #475569; }");
    connect(refreshBtn, &QPushButton::clicked, this, &InventoryTab::refresh);
    topLayout->addWidget(refreshBtn);

    mainLayout->addLayout(topLayout);

    statusLabel_ = new QLabel(this);
    statusLabel_->setStyleSheet("font-size: 12px; font-weight: 500;");
    statusLabel_->setVisible(false);
    mainLayout->addWidget(statusLabel_);

    table_ = new QTableWidget(this);
    table_->setColumnCount(7);
    table_->setHorizontalHeaderLabels({"ID", "Barcode", "Acronym", "Product Name", "Unit", "Current Stock", "Adjust Stock"});
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setStyleSheet(
        "QTableWidget { background-color: #ffffff; color: #0f172a; border: 1px solid #e2e8f0; border-radius: 6px; }"
        "QHeaderView::section { background-color: #f1f5f9; color: #334155; font-weight: bold; padding: 6px; border: none; }"
    );
    mainLayout->addWidget(table_);
}

void InventoryTab::refresh() {
    onSearchFilterChanged(searchEdit_->text());
}

void InventoryTab::onSearchFilterChanged(const QString& filter) {
    auto all = productService_.allProducts();
    QString query = filter.trimmed().toLower();

    filteredProducts_.clear();
    for (const auto& p : all) {
        if (query.isEmpty() ||
            QString::fromStdString(p.barcode).toLower().contains(query) ||
            QString::fromStdString(p.acronym).toLower().contains(query) ||
            QString::fromStdString(p.name).toLower().contains(query)) {
            filteredProducts_.push_back(p);
        }
    }

    populateTable();
}

void InventoryTab::populateTable() {
    table_->setRowCount(static_cast<int>(filteredProducts_.size()));

    for (int i = 0; i < static_cast<int>(filteredProducts_.size()); ++i) {
        const auto& p = filteredProducts_[i];

        auto* idItem = new QTableWidgetItem(QString::number(p.id));
        idItem->setTextAlignment(Qt::AlignCenter);
        table_->setItem(i, 0, idItem);

        auto* bcItem = new QTableWidgetItem(QString::fromStdString(p.barcode));
        bcItem->setTextAlignment(Qt::AlignCenter);
        table_->setItem(i, 1, bcItem);

        auto* acroItem = new QTableWidgetItem(QString::fromStdString(p.acronym));
        acroItem->setTextAlignment(Qt::AlignCenter);
        table_->setItem(i, 2, acroItem);

        table_->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(p.name)));

        auto* unitItem = new QTableWidgetItem(QString::fromStdString(unitToText(p.unit)));
        unitItem->setTextAlignment(Qt::AlignCenter);
        table_->setItem(i, 4, unitItem);

        auto* stockItem = new QTableWidgetItem(QString::fromStdString(quantityToText(p.availableQty)));
        stockItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        stockItem->setFont(QFont("Segoe UI", 10, QFont::Bold));
        table_->setItem(i, 5, stockItem);

        // Cell widget with spinbox and button
        auto* adjustWidget = new QWidget(this);
        auto* adjLayout = new QHBoxLayout(adjustWidget);
        adjLayout->setContentsMargins(4, 2, 4, 2);
        adjLayout->setSpacing(6);

        auto* spin = new QDoubleSpinBox(adjustWidget);
        spin->setRange(0.0, 999999.0);
        spin->setValue(p.availableQty);
        spin->setFixedWidth(90);
        spin->setStyleSheet("QDoubleSpinBox { padding: 3px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; }");
        adjLayout->addWidget(spin);

        auto* updateBtn = new QPushButton("Update", adjustWidget);
        updateBtn->setStyleSheet("QPushButton { padding: 4px 10px; background: #059669; color: #ffffff; font-weight: bold; border-radius: 4px; border: none; } QPushButton:hover { background: #047857; }");
        connect(updateBtn, &QPushButton::clicked, [this, i, spin]() {
            try {
                if (i >= 0 && i < static_cast<int>(filteredProducts_.size())) {
                    int pId = filteredProducts_[i].id;
                    double newStock = spin->value();
                    productService_.updateStock(pId, newStock);

                    statusLabel_->setText(QString("Stock updated for %1 to %2")
                                              .arg(QString::fromStdString(filteredProducts_[i].acronym))
                                              .arg(newStock));
                    statusLabel_->setStyleSheet("color: #16a34a; font-weight: bold;");
                    statusLabel_->setVisible(true);

                    refresh();
                    emit inventoryChanged();
                }
            } catch (const PosError& e) {
                statusLabel_->setText(QString::fromStdString(e.what()));
                statusLabel_->setStyleSheet("color: #dc2626; font-weight: bold;");
                statusLabel_->setVisible(true);
            }
        });
        adjLayout->addWidget(updateBtn);

        table_->setCellWidget(i, 6, adjustWidget);
    }
}

void InventoryTab::onUpdateStockRow(int /*row*/) {
    // Handled by cell widget lambdas
}
