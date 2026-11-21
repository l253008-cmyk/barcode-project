#pragma once

#include <QString>
#include <QWidget>

#include "pos/domain/Product.hpp"

class QLabel;

class ProductDetailsWidget : public QWidget {
    Q_OBJECT

public:
    explicit ProductDetailsWidget(QWidget* parent = nullptr);

    void showProduct(const Product& product);
    void showDescription(const QString& text);
    void showMessage(const QString& text);
    void clear();

private:
    static const int kFieldCount = 10;
    QLabel* description_;
    QLabel* values_[kFieldCount];
};
