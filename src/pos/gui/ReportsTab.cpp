// ReportsTab.cpp                                  Owner: Person 4
// Manager UI tab for viewing 6 managerial business reports.

#include "pos/gui/ReportsTab.hpp"
#include "pos/common/DateUtils.hpp"
#include "pos/common/PosError.hpp"

#include <QDate>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>

ReportsTab::ReportsTab(ReportService& reportService, QWidget* parent)
    : QWidget(parent), reportService_(reportService) {
    setupUi();
    reportTitleLabel_->setText("Select a report from the dropdown above and click 'Generate Report'.");
    reportTitleLabel_->setStyleSheet("font-size: 13px; font-weight: 500; color: #64748b; font-style: italic; padding: 6px 0;");
}

void ReportsTab::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(14, 14, 14, 14);

    // Controls bar
    auto* controlsLayout = new QHBoxLayout();
    controlsLayout->setSpacing(10);

    auto* typeLabel = new QLabel("Select Report:", this);
    typeLabel->setStyleSheet("font-weight: bold; color: #1e293b;");
    controlsLayout->addWidget(typeLabel);

    reportCombo_ = new QComboBox(this);
    reportCombo_->addItem("1. Incomplete / Pending Orders");
    reportCombo_->addItem("2. Completed (Paid) Orders");
    reportCombo_->addItem("3. Cash Sales by Period");
    reportCombo_->addItem("4. All Sales with Discounts");
    reportCombo_->addItem("5. Top 10 Most Sold Items");
    reportCombo_->addItem("6. Top 10 Items by Remaining Stock");
    reportCombo_->setStyleSheet("QComboBox { padding: 6px 12px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; font-weight: 500; }");
    connect(reportCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReportsTab::onReportTypeChanged);
    controlsLayout->addWidget(reportCombo_);

    // Date range filter for Cash Sales
    dateFilterWidget_ = new QWidget(this);
    auto* dfLayout = new QHBoxLayout(dateFilterWidget_);
    dfLayout->setContentsMargins(0, 0, 0, 0);
    dfLayout->setSpacing(6);

    auto* startLabel = new QLabel("From:", dateFilterWidget_);
    startLabel->setStyleSheet("color: #475569;");
    dfLayout->addWidget(startLabel);

    startDateEdit_ = new QLineEdit(dateFilterWidget_);
    startDateEdit_->setText(QDate::currentDate().addDays(-30).toString("yyyy-MM-dd"));
    startDateEdit_->setFixedWidth(100);
    startDateEdit_->setStyleSheet("QLineEdit { padding: 5px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; }");
    dfLayout->addWidget(startDateEdit_);

    auto* endLabel = new QLabel("To:", dateFilterWidget_);
    endLabel->setStyleSheet("color: #475569;");
    dfLayout->addWidget(endLabel);

    endDateEdit_ = new QLineEdit(dateFilterWidget_);
    endDateEdit_->setText(QDate::currentDate().toString("yyyy-MM-dd"));
    endDateEdit_->setFixedWidth(100);
    endDateEdit_->setStyleSheet("QLineEdit { padding: 5px; border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; color: #0f172a; }");
    dfLayout->addWidget(endDateEdit_);

    dateFilterWidget_->setVisible(false); // Only visible for report index 2
    controlsLayout->addWidget(dateFilterWidget_);

    auto* runBtn = new QPushButton("Generate Report", this);
    runBtn->setStyleSheet("QPushButton { padding: 6px 16px; background: #2563eb; color: #ffffff; font-weight: bold; border-radius: 4px; border: none; } QPushButton:hover { background: #1d4ed8; }");
    connect(runBtn, &QPushButton::clicked, this, &ReportsTab::onGenerateReport);
    controlsLayout->addWidget(runBtn);

    controlsLayout->addStretch();
    mainLayout->addLayout(controlsLayout);

    // Title label
    reportTitleLabel_ = new QLabel(this);
    reportTitleLabel_->setStyleSheet("font-size: 15px; font-weight: bold; color: #0f172a; padding: 4px 0;");
    mainLayout->addWidget(reportTitleLabel_);

    // Table
    table_ = new QTableWidget(this);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setStyleSheet(
        "QTableWidget { background-color: #ffffff; color: #0f172a; border: 1px solid #e2e8f0; border-radius: 6px; }"
        "QHeaderView::section { background-color: #f1f5f9; color: #334155; font-weight: bold; padding: 6px; border: none; }"
    );
    mainLayout->addWidget(table_);
}

void ReportsTab::refresh() {
    if (table_->rowCount() > 0) {
        onGenerateReport();
    }
}

void ReportsTab::onReportTypeChanged(int index) {
    dateFilterWidget_->setVisible(index == 2);
}

void ReportsTab::onGenerateReport() {
    int index = reportCombo_->currentIndex();
    ReportTable rep;

    switch (index) {
    case 0:
        rep = reportService_.incompleteOrders();
        break;
    case 1:
        rep = reportService_.completedOrders();
        break;
    case 2: {
        QString startStr = startDateEdit_->text().trimmed();
        QString endStr = endDateEdit_->text().trimmed();

        if (startStr.isEmpty() || endStr.isEmpty()) {
            QMessageBox::warning(this, "Missing Dates", "Both Start Date and End Date are required for period report.");
            return;
        }

        std::string start = startStr.toStdString();
        std::string end = endStr.toStdString();

        if (!pos::DateUtils::isValidDate(start)) {
            QMessageBox::warning(this, "Invalid Start Date",
                QString("Start Date '%1' is invalid.\nExpected valid calendar date in YYYY-MM-DD format (e.g. 2026-11-23).").arg(startStr));
            return;
        }

        if (!pos::DateUtils::isValidDate(end)) {
            QMessageBox::warning(this, "Invalid End Date",
                QString("End Date '%1' is invalid.\nExpected valid calendar date in YYYY-MM-DD format (e.g. 2026-12-31).").arg(endStr));
            return;
        }

        if (end < start) {
            QMessageBox::warning(this, "Invalid Date Range",
                QString("End Date '%1' cannot precede Start Date '%2'.").arg(endStr, startStr));
            return;
        }

        try {
            rep = reportService_.cashSalesByPeriod(start, end);
        }
        catch (const PosError& e) {
            QMessageBox::warning(this, "Report Error", QString::fromStdString(e.what()));
            return;
        }
        break;
    }
    case 3:
        rep = reportService_.allSalesWithDiscounts();
        break;
    case 4:
        rep = reportService_.top10MostSoldItems();
        break;
    case 5:
        rep = reportService_.top10ByRemainingStock();
        break;
    default:
        break;
    }

    renderReport(rep);
}

void ReportsTab::renderReport(const ReportTable& rep) {
    reportTitleLabel_->setText(QString::fromStdString(rep.title));
    reportTitleLabel_->setStyleSheet("font-size: 15px; font-weight: bold; color: #0f172a; padding: 4px 0;");

    int colCount = static_cast<int>(rep.headers.size());
    table_->setColumnCount(colCount);

    QStringList headerList;
    for (const auto& h : rep.headers) {
        headerList << QString::fromStdString(h);
    }
    table_->setHorizontalHeaderLabels(headerList);

    for (int c = 0; c < colCount; ++c) {
        table_->horizontalHeader()->setSectionResizeMode(c, QHeaderView::Stretch);
    }

    int rowCount = static_cast<int>(rep.rows.size());
    bool hasTotals = !rep.totalsRow.empty();
    table_->setRowCount(rowCount + (hasTotals ? 1 : 0));

    for (int r = 0; r < rowCount; ++r) {
        const auto& rowData = rep.rows[r];
        for (int c = 0; c < colCount && c < static_cast<int>(rowData.size()); ++c) {
            auto* item = new QTableWidgetItem(QString::fromStdString(rowData[c]));
            item->setTextAlignment(Qt::AlignCenter);
            table_->setItem(r, c, item);
        }
    }

    // Render totals row if available
    if (hasTotals) {
        int totRow = rowCount;
        for (int c = 0; c < colCount && c < static_cast<int>(rep.totalsRow.size()); ++c) {
            auto* item = new QTableWidgetItem(QString::fromStdString(rep.totalsRow[c]));
            item->setTextAlignment(Qt::AlignCenter);
            item->setFont(QFont("Segoe UI", 9, QFont::Bold));
            item->setBackground(QColor("#f1f5f9"));
            item->setForeground(QColor("#0f172a"));
            table_->setItem(totRow, c, item);
        }
    }
}