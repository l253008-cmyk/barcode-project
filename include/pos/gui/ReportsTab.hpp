// ReportsTab.hpp                                  Owner: Person 4
// Manager UI tab for viewing 6 managerial business reports.

#ifndef POS_REPORTS_TAB_HPP
#define POS_REPORTS_TAB_HPP

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>

#include "pos/services/ReportService.hpp"

class ReportsTab : public QWidget {
    Q_OBJECT

public:
    explicit ReportsTab(ReportService& reportService, QWidget* parent = nullptr);

    void refresh();

private slots:
    void onReportTypeChanged(int index);
    void onGenerateReport();

private:
    void setupUi();
    void renderReport(const ReportTable& table);

    ReportService& reportService_;
    QComboBox* reportCombo_ = nullptr;
    QWidget* dateFilterWidget_ = nullptr;
    QLineEdit* startDateEdit_ = nullptr;
    QLineEdit* endDateEdit_ = nullptr;
    QLabel* reportTitleLabel_ = nullptr;
    QTableWidget* table_ = nullptr;
};

#endif // POS_REPORTS_TAB_HPP