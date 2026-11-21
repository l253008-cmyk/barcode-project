// CategoryTab.cpp                                  Owner: Person 3
// Manager UI tab for configuring product categories (PCategory.txt).

#include "pos/gui/CategoryTab.hpp"
#include "pos/common/PosError.hpp"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QVBoxLayout>

CategoryTab::CategoryTab(CategoryService& categoryService, QWidget* parent)
    : QWidget(parent), categoryService_(categoryService) {
    setupUi();
    refresh();
}

void CategoryTab::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // Form box
    auto* formBox = new QGroupBox("Add New Category", this);
    formBox->setStyleSheet("QGroupBox { font-weight: bold; font-size: 13px; color: #1e293b; }");
    auto* formLayout = new QHBoxLayout(formBox);
    formLayout->setSpacing(12);

    idEdit_ = new QLineEdit(formBox);
    idEdit_->setPlaceholderText("ID (10-99)");
    idEdit_->setMaxLength(2);
    idEdit_->setFixedWidth(80);
    idEdit_->setStyleSheet("QLineEdit { padding: 6px; border: 1px solid #cbd5e1; border-radius: 4px; color: #0f172a; background: #ffffff; }");
    formLayout->addWidget(idEdit_);

    nameEdit_ = new QLineEdit(formBox);
    nameEdit_->setPlaceholderText("Category Name (e.g. Beverages)");
    nameEdit_->setStyleSheet("QLineEdit { padding: 6px; border: 1px solid #cbd5e1; border-radius: 4px; color: #0f172a; background: #ffffff; }");
    formLayout->addWidget(nameEdit_);

    acronymEdit_ = new QLineEdit(formBox);
    acronymEdit_->setPlaceholderText("Acronym (e.g. BEV)");
    acronymEdit_->setMaxLength(10);
    acronymEdit_->setFixedWidth(100);
    acronymEdit_->setStyleSheet("QLineEdit { padding: 6px; border: 1px solid #cbd5e1; border-radius: 4px; color: #0f172a; background: #ffffff; }");
    formLayout->addWidget(acronymEdit_);

    auto* addBtn = new QPushButton("Add Category", formBox);
    addBtn->setStyleSheet("QPushButton { padding: 7px 16px; background: #2563eb; color: #ffffff; font-weight: bold; border-radius: 4px; border: none; } QPushButton:hover { background: #1d4ed8; }");
    connect(addBtn, &QPushButton::clicked, this, &CategoryTab::onAddCategory);
    formLayout->addWidget(addBtn);

    mainLayout->addWidget(formBox);

    statusLabel_ = new QLabel(this);
    statusLabel_->setStyleSheet("font-size: 12px; font-weight: 500;");
    statusLabel_->setVisible(false);
    mainLayout->addWidget(statusLabel_);

    // Table
    table_ = new QTableWidget(this);
    table_->setColumnCount(3);
    table_->setHorizontalHeaderLabels({"Category ID", "Category Name", "Acronym"});
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setStyleSheet(
        "QTableWidget { background-color: #ffffff; color: #0f172a; border: 1px solid #e2e8f0; border-radius: 6px; }"
        "QHeaderView::section { background-color: #f1f5f9; color: #334155; font-weight: bold; padding: 6px; border: none; }"
    );
    mainLayout->addWidget(table_);
}

void CategoryTab::refresh() {
    auto categories = categoryService_.all();
    table_->setRowCount(static_cast<int>(categories.size()));

    for (int i = 0; i < static_cast<int>(categories.size()); ++i) {
        const auto& cat = categories[i];
        auto* idItem = new QTableWidgetItem(QString::fromStdString(cat.id));
        idItem->setTextAlignment(Qt::AlignCenter);
        table_->setItem(i, 0, idItem);

        table_->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(cat.name)));

        auto* acroItem = new QTableWidgetItem(QString::fromStdString(cat.acronym));
        acroItem->setTextAlignment(Qt::AlignCenter);
        table_->setItem(i, 2, acroItem);
    }
}

void CategoryTab::onAddCategory() {
    std::string id = idEdit_->text().trimmed().toStdString();
    std::string name = nameEdit_->text().trimmed().toStdString();
    std::string acronym = acronymEdit_->text().trimmed().toStdString();

    try {
        categoryService_.add(id, name, acronym);
        statusLabel_->setText("Category added successfully!");
        statusLabel_->setStyleSheet("color: #16a34a; font-weight: bold;");
        statusLabel_->setVisible(true);

        idEdit_->clear();
        nameEdit_->clear();
        acronymEdit_->clear();

        refresh();
        emit categoriesChanged();
    } catch (const PosError& e) {
        statusLabel_->setText(QString::fromStdString(e.what()));
        statusLabel_->setStyleSheet("color: #dc2626; font-weight: bold;");
        statusLabel_->setVisible(true);
    }
}
