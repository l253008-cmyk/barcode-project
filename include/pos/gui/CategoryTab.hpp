// CategoryTab.hpp                                  Owner: Person 3
// Manager UI tab for configuring product categories (PCategory.txt).

#ifndef POS_CATEGORY_TAB_HPP
#define POS_CATEGORY_TAB_HPP

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>

#include "pos/services/CategoryService.hpp"

class CategoryTab : public QWidget {
    Q_OBJECT

public:
    explicit CategoryTab(CategoryService& categoryService, QWidget* parent = nullptr);

    void refresh();

signals:
    void categoriesChanged();

private slots:
    void onAddCategory();

private:
    void setupUi();

    CategoryService& categoryService_;
    QTableWidget* table_ = nullptr;
    QLineEdit* idEdit_ = nullptr;
    QLineEdit* nameEdit_ = nullptr;
    QLineEdit* acronymEdit_ = nullptr;
    QLabel* statusLabel_ = nullptr;
};

#endif // POS_CATEGORY_TAB_HPP
