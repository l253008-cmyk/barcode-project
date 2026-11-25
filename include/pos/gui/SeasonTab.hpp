// SeasonTab.hpp                                    Owner: Person 3
// Manager UI tab for configuring seasonal discounts (Seasons.txt).

#ifndef POS_SEASON_TAB_HPP
#define POS_SEASON_TAB_HPP

#include <QWidget>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>

#include "pos/services/SeasonService.hpp"

class SeasonTab : public QWidget {
    Q_OBJECT

public:
    explicit SeasonTab(SeasonService& seasonService, QWidget* parent = nullptr);

    void refresh();

private slots:
    void onAddSeason();
    void onToggleActive(int row);

private:
    void setupUi();

    SeasonService& seasonService_;
    QTableWidget* table_ = nullptr;
    QLineEdit* startDateEdit_ = nullptr;
    QLineEdit* endDateEdit_ = nullptr;
    QDoubleSpinBox* percentSpin_ = nullptr;
    QLabel* statusLabel_ = nullptr;
};

#endif // POS_SEASON_TAB_HPP