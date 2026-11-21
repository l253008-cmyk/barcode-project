// SeasonTab.cpp                                    Owner: Person 3
// Manager UI tab for configuring seasonal discounts (Seasons.txt).

#include "pos/gui/SeasonTab.hpp"
#include "pos/common/PosError.hpp"

#include <QDate>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QVBoxLayout>

SeasonTab::SeasonTab(SeasonService& seasonService, QWidget* parent)
    : QWidget(parent), seasonService_(seasonService) {
    setupUi();
    refresh();
}

void SeasonTab::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // Form box
    auto* formBox = new QGroupBox("Add Seasonal Discount Period", this);
    formBox->setStyleSheet("QGroupBox { font-weight: bold; font-size: 13px; color: #1e293b; }");
    auto* formLayout = new QHBoxLayout(formBox);
    formLayout->setSpacing(12);

    startDateEdit_ = new QLineEdit(formBox);
    startDateEdit_->setPlaceholderText("Start (YYYY-MM-DD)");
    startDateEdit_->setText(QDate::currentDate().toString("yyyy-MM-dd"));
    startDateEdit_->setStyleSheet("QLineEdit { padding: 6px; border: 1px solid #cbd5e1; border-radius: 4px; color: #0f172a; background: #ffffff; }");
    formLayout->addWidget(startDateEdit_);

    endDateEdit_ = new QLineEdit(formBox);
    endDateEdit_->setPlaceholderText("End (YYYY-MM-DD)");
    endDateEdit_->setText(QDate::currentDate().addDays(30).toString("yyyy-MM-dd"));
    endDateEdit_->setStyleSheet("QLineEdit { padding: 6px; border: 1px solid #cbd5e1; border-radius: 4px; color: #0f172a; background: #ffffff; }");
    formLayout->addWidget(endDateEdit_);

    percentSpin_ = new QDoubleSpinBox(formBox);
    percentSpin_->setRange(0.1, 100.0);
    percentSpin_->setValue(10.0);
    percentSpin_->setSuffix(" %");
    percentSpin_->setFixedWidth(100);
    percentSpin_->setStyleSheet("QDoubleSpinBox { padding: 6px; border: 1px solid #cbd5e1; border-radius: 4px; color: #0f172a; background: #ffffff; }");
    formLayout->addWidget(percentSpin_);

    auto* addBtn = new QPushButton("Add Season", formBox);
    addBtn->setStyleSheet("QPushButton { padding: 7px 16px; background: #2563eb; color: #ffffff; font-weight: bold; border-radius: 4px; border: none; } QPushButton:hover { background: #1d4ed8; }");
    connect(addBtn, &QPushButton::clicked, this, &SeasonTab::onAddSeason);
    formLayout->addWidget(addBtn);

    mainLayout->addWidget(formBox);

    statusLabel_ = new QLabel(this);
    statusLabel_->setStyleSheet("font-size: 12px; font-weight: 500;");
    statusLabel_->setVisible(false);
    mainLayout->addWidget(statusLabel_);

    // Table
    table_ = new QTableWidget(this);
    table_->setColumnCount(5);
    table_->setHorizontalHeaderLabels({"Start Date", "End Date", "Discount %", "Status", "Action"});
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setStyleSheet(
        "QTableWidget { background-color: #ffffff; color: #0f172a; border: 1px solid #e2e8f0; border-radius: 6px; }"
        "QHeaderView::section { background-color: #f1f5f9; color: #334155; font-weight: bold; padding: 6px; border: none; }"
    );
    mainLayout->addWidget(table_);
}

void SeasonTab::refresh() {
    auto seasons = seasonService_.all();
    table_->setRowCount(static_cast<int>(seasons.size()));

    for (int i = 0; i < static_cast<int>(seasons.size()); ++i) {
        const auto& s = seasons[i];

        auto* sItem = new QTableWidgetItem(QString::fromStdString(s.startDate));
        sItem->setTextAlignment(Qt::AlignCenter);
        table_->setItem(i, 0, sItem);

        auto* eItem = new QTableWidgetItem(QString::fromStdString(s.endDate));
        eItem->setTextAlignment(Qt::AlignCenter);
        table_->setItem(i, 1, eItem);

        auto* pItem = new QTableWidgetItem(QString::number(s.percent, 'f', 1) + "%");
        pItem->setTextAlignment(Qt::AlignCenter);
        table_->setItem(i, 2, pItem);

        auto* stItem = new QTableWidgetItem(s.active ? "Active" : "Inactive");
        stItem->setTextAlignment(Qt::AlignCenter);
        stItem->setForeground(s.active ? QColor("#16a34a") : QColor("#dc2626"));
        table_->setItem(i, 3, stItem);

        auto* toggleBtn = new QPushButton(s.active ? "Deactivate" : "Activate", this);
        toggleBtn->setStyleSheet(
            s.active
                ? "QPushButton { padding: 4px 10px; background: #fee2e2; color: #b91c1c; border: 1px solid #fca5a5; border-radius: 4px; font-weight: 500; }"
                : "QPushButton { padding: 4px 10px; background: #dcfce7; color: #15803d; border: 1px solid #86efac; border-radius: 4px; font-weight: 500; }"
        );
        connect(toggleBtn, &QPushButton::clicked, [this, i]() {
            onToggleActive(i);
        });
        table_->setCellWidget(i, 4, toggleBtn);
    }
}

void SeasonTab::onAddSeason() {
    std::string start = startDateEdit_->text().trimmed().toStdString();
    std::string end = endDateEdit_->text().trimmed().toStdString();
    double percent = percentSpin_->value();

    try {
        seasonService_.add(start, end, percent);
        statusLabel_->setText("Seasonal discount added successfully!");
        statusLabel_->setStyleSheet("color: #16a34a; font-weight: bold;");
        statusLabel_->setVisible(true);

        refresh();
    } catch (const PosError& e) {
        statusLabel_->setText(QString::fromStdString(e.what()));
        statusLabel_->setStyleSheet("color: #dc2626; font-weight: bold;");
        statusLabel_->setVisible(true);
    }
}

void SeasonTab::onToggleActive(int row) {
    auto seasons = seasonService_.all();
    if (row >= 0 && row < static_cast<int>(seasons.size())) {
        seasonService_.setActive(row, !seasons[row].active);
        refresh();
    }
}
