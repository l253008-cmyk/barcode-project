// SeasonTab.cpp                                    Owner: Person 3
// Manager UI tab for configuring seasonal discounts (Seasons.txt).

#include "pos/gui/SeasonTab.hpp"
#include "pos/common/DateUtils.hpp"
#include "pos/common/PosError.hpp"

#include <QDate>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
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
    auto* formLayout = new QGridLayout(formBox);
    formLayout->setSpacing(10);

    auto* startLbl = new QLabel("Start Date (YYYY-MM-DD):", formBox);
    startLbl->setStyleSheet("font-weight: bold; color: #0f172a;");
    formLayout->addWidget(startLbl, 0, 0);

    startDateEdit_ = new QLineEdit(formBox);
    startDateEdit_->setPlaceholderText("YYYY-MM-DD");
    startDateEdit_->setText(QDate::currentDate().toString("yyyy-MM-dd"));
    startDateEdit_->setStyleSheet("QLineEdit { padding: 6px; border: 1.5px solid #cbd5e1; border-radius: 4px; color: #0f172a; background: #ffffff; font-weight: 500; }");
    formLayout->addWidget(startDateEdit_, 0, 1);

    auto* endLbl = new QLabel("End Date (YYYY-MM-DD):", formBox);
    endLbl->setStyleSheet("font-weight: bold; color: #0f172a;");
    formLayout->addWidget(endLbl, 0, 2);

    endDateEdit_ = new QLineEdit(formBox);
    endDateEdit_->setPlaceholderText("YYYY-MM-DD");
    endDateEdit_->setText(QDate::currentDate().addDays(30).toString("yyyy-MM-dd"));
    endDateEdit_->setStyleSheet("QLineEdit { padding: 6px; border: 1.5px solid #cbd5e1; border-radius: 4px; color: #0f172a; background: #ffffff; font-weight: 500; }");
    formLayout->addWidget(endDateEdit_, 0, 3);

    auto* discLbl = new QLabel("Discount (%):", formBox);
    discLbl->setStyleSheet("font-weight: bold; color: #0f172a;");
    formLayout->addWidget(discLbl, 1, 0);

    percentSpin_ = new QDoubleSpinBox(formBox);
    percentSpin_->setRange(0.1, 100.0);
    percentSpin_->setValue(10.0);
    percentSpin_->setSuffix(" %");
    formLayout->addWidget(percentSpin_, 1, 1);

    auto* addBtn = new QPushButton("➕ Add Seasonal Discount", formBox);
    addBtn->setStyleSheet("QPushButton { padding: 8px 18px; background: #2563eb; color: #ffffff; font-weight: bold; border-radius: 4px; border: none; } QPushButton:hover { background: #1d4ed8; }");
    connect(addBtn, &QPushButton::clicked, this, &SeasonTab::onAddSeason);
    formLayout->addWidget(addBtn, 1, 3);

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

    std::string today = QDate::currentDate().toString("yyyy-MM-dd").toStdString();

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

        QString statusText;
        QColor statusColor;
        if (!s.active) {
            statusText = "Disabled";
            statusColor = QColor("#64748b");
        } else if (today < s.startDate) {
            statusText = "⏳ Auto on " + QString::fromStdString(s.startDate);
            statusColor = QColor("#d97706");
        } else if (today > s.endDate) {
            statusText = "Expired";
            statusColor = QColor("#94a3b8");
        } else {
            statusText = "🟢 Active Now";
            statusColor = QColor("#16a34a");
        }

        auto* stItem = new QTableWidgetItem(statusText);
        stItem->setTextAlignment(Qt::AlignCenter);
        stItem->setForeground(statusColor);
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
    QString startStr = startDateEdit_->text().trimmed();
    QString endStr = endDateEdit_->text().trimmed();
    double percent = percentSpin_->value();

    if (startStr.isEmpty() || endStr.isEmpty()) {
        QMessageBox::warning(this, "Missing Dates", "Both Start Date and End Date are required.");
        statusLabel_->setText("Error: Start and End dates cannot be empty.");
        statusLabel_->setStyleSheet("color: #dc2626; font-weight: bold;");
        statusLabel_->setVisible(true);
        return;
    }

    std::string start = startStr.toStdString();
    std::string end = endStr.toStdString();

    if (!pos::DateUtils::isValidDate(start)) {
        QMessageBox::warning(this, "Invalid Start Date",
            QString("Start Date '%1' is invalid.\nExpected valid calendar date in YYYY-MM-DD format (e.g. 2026-11-23).").arg(startStr));
        statusLabel_->setText("Error: Invalid Start Date format or calendar day.");
        statusLabel_->setStyleSheet("color: #dc2626; font-weight: bold;");
        statusLabel_->setVisible(true);
        return;
    }

    if (!pos::DateUtils::isValidDate(end)) {
        QMessageBox::warning(this, "Invalid End Date",
            QString("End Date '%1' is invalid.\nExpected valid calendar date in YYYY-MM-DD format (e.g. 2026-12-31).").arg(endStr));
        statusLabel_->setText("Error: Invalid End Date format or calendar day.");
        statusLabel_->setStyleSheet("color: #dc2626; font-weight: bold;");
        statusLabel_->setVisible(true);
        return;
    }

    if (end < start) {
        QMessageBox::warning(this, "Invalid Date Range",
            QString("End Date '%1' cannot precede Start Date '%2'.").arg(endStr, startStr));
        statusLabel_->setText("Error: End Date cannot precede Start Date.");
        statusLabel_->setStyleSheet("color: #dc2626; font-weight: bold;");
        statusLabel_->setVisible(true);
        return;
    }

    std::string today = QDate::currentDate().toString("yyyy-MM-dd").toStdString();
    try {
        seasonService_.add(start, end, percent);
        if (start > today) {
            statusLabel_->setText(QString("Seasonal discount scheduled! It will automatically activate on %1.").arg(startStr));
            statusLabel_->setStyleSheet("color: #2563eb; font-weight: bold;");
        } else {
            statusLabel_->setText("Seasonal discount added and active now!");
            statusLabel_->setStyleSheet("color: #16a34a; font-weight: bold;");
        }
        statusLabel_->setVisible(true);

        refresh();
    } catch (const PosError& e) {
        QMessageBox::warning(this, "Cannot Add Season", QString::fromStdString(e.what()));
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
