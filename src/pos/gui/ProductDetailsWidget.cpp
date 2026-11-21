#include "pos/gui/ProductDetailsWidget.hpp"

#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

ProductDetailsWidget::ProductDetailsWidget(QWidget* parent)
    : QWidget(parent) {
    const char* names[kFieldCount] = {"Id",       "Barcode", "Acronym", "Name",      "Unit",
                                      "Unit price", "Tax",   "Discount", "Available", "Barcode file"};

    description_ = new QLabel(this);
    description_->setWordWrap(true);

    QFormLayout* form = new QFormLayout();
    for (int i = 0; i < kFieldCount; ++i) {
        values_[i] = new QLabel("-", this);
        values_[i]->setTextInteractionFlags(Qt::TextSelectableByMouse);
        values_[i]->setStyleSheet("color: #0F172A; font-weight: bold; font-size: 13px;");
        QLabel* label = new QLabel(QString(names[i]) + ":", this);
        label->setStyleSheet("color: #334155; font-weight: 600; font-size: 13px;");
        form->addRow(label, values_[i]);
    }

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(description_);
    layout->addLayout(form);
    layout->addStretch();
}

void ProductDetailsWidget::clear() {
    description_->setText("");
    description_->setStyleSheet("");
    for (int i = 0; i < kFieldCount; ++i) {
        values_[i]->setText("-");
    }
}

void ProductDetailsWidget::showProduct(const Product& product) {
    clear();
    values_[0]->setText(QString::number(product.id));
    values_[1]->setText(QString::fromStdString(product.barcode));
    values_[2]->setText(QString::fromStdString(product.acronym));
    values_[3]->setText(QString::fromStdString(product.name));
    values_[4]->setText(product.unit == MeasurementUnit::Count ? "count" : "weight");
    values_[5]->setText(QString::fromStdString(product.unitPrice.toString()));
    values_[6]->setText(QString::fromStdString(product.tax.toString()));
    values_[7]->setText(QString::fromStdString(product.discount.toString()));
    values_[8]->setText(QString::number(product.availableQty));
    values_[9]->setText(QString::fromStdString(product.barcodePath));
}

void ProductDetailsWidget::showDescription(const QString& text) {
    clear();
    description_->setText(text);
}

void ProductDetailsWidget::showMessage(const QString& text) {
    clear();
    description_->setStyleSheet("color: #b00020;");
    description_->setText(text);
}
