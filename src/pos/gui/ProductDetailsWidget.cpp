#include "pos/gui/ProductDetailsWidget.hpp"

#include <QLabel>
#include <QVBoxLayout>

ProductDetailsWidget::ProductDetailsWidget(QWidget* parent)
    : QWidget(parent) {
    description_ = new QLabel(this);
    description_->setWordWrap(true);
    description_->setStyleSheet("color: #0F172A; font-size: 13px; font-weight: 500;");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 4, 0, 4);
    layout->addWidget(description_);
    layout->addStretch();
}

void ProductDetailsWidget::clear() {
    description_->setText("");
    description_->setStyleSheet("color: #0F172A; font-size: 13px; font-weight: 500;");
}

void ProductDetailsWidget::showProduct(const Product& product) {
    clear();
    QString text = QString(
        "Name: %1\n"
        "Acronym: %2\n"
        "Barcode: %3\n"
        "Unit: %4\n"
        "Unit price: %5\n"
        "Tax: %6\n"
        "Discount: %7\n"
        "Available stock: %8"
    ).arg(
        QString::fromStdString(product.name),
        QString::fromStdString(product.acronym),
        QString::fromStdString(product.barcode),
        product.unit == MeasurementUnit::Count ? "count" : "weight",
        QString::fromStdString(product.unitPrice.toString()),
        QString::fromStdString(product.tax.toString()),
        QString::fromStdString(product.discount.toString()),
        QString::number(product.availableQty)
    );
    description_->setText(text);
}

void ProductDetailsWidget::showDescription(const QString& text) {
    clear();
    description_->setText(text);
}

void ProductDetailsWidget::showMessage(const QString& text) {
    clear();
    description_->setStyleSheet("color: #dc2626; font-size: 13px; font-weight: bold;");
    description_->setText(text);
}
