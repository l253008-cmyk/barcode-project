// ManagerScreen.cpp                                   Owner: Person 1
// Comprehensive Manager Workstation hosting Categories, Products, Inventory, Seasons, and Reports.

#include "pos/gui/ManagerScreen.hpp"
#include "pos/gui/CategoryTab.hpp"
#include "pos/gui/ProductTab.hpp"
#include "pos/gui/InventoryTab.hpp"
#include "pos/gui/SeasonTab.hpp"
#include "pos/gui/ReportsTab.hpp"

#include <QVBoxLayout>

ManagerScreen::ManagerScreen(CategoryService& categories,
    ProductService& products,
    SeasonService& seasons,
    ReportService& reports,
    IBarcodeReader* reader,
    QWidget* parent)
    : QWidget(parent),
    categoryService_(categories),
    productService_(products),
    seasonService_(seasons),
    reportService_(reports),
    reader_(reader) {
    setupUi();
}

void ManagerScreen::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);

    tabs_ = new QTabWidget(this);
    tabs_->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #cbd5e1; background: #ffffff; border-radius: 6px; }"
        "QTabBar::tab {"
        "  background: #f1f5f9;"
        "  color: #334155;"
        "  font-weight: bold;"
        "  padding: 8px 18px;"
        "  margin-right: 2px;"
        "  border-top-left-radius: 6px;"
        "  border-top-right-radius: 6px;"
        "}"
        "QTabBar::tab:selected {"
        "  background: #ffffff;"
        "  color: #2563eb;"
        "  border-bottom: 2px solid #2563eb;"
        "}"
        "QTabBar::tab:hover:!selected {"
        "  background: #e2e8f0;"
        "}"
    );

    categoryTab_ = new CategoryTab(categoryService_, tabs_);
    productTab_ = new ProductTab(productService_, categoryService_, tabs_);
    inventoryTab_ = new InventoryTab(productService_, tabs_);
    seasonTab_ = new SeasonTab(seasonService_, tabs_);
    reportsTab_ = new ReportsTab(reportService_, tabs_);

    tabs_->addTab(categoryTab_, "?? Categories");
    tabs_->addTab(productTab_, "??? Products & Barcodes");
    tabs_->addTab(inventoryTab_, "?? Stock Adjustment");
    tabs_->addTab(seasonTab_, "?? Seasonal Discounts");
    tabs_->addTab(reportsTab_, "?? Reports");

    // Cross-tab synchronization
    connect(categoryTab_, &CategoryTab::categoriesChanged, productTab_, &ProductTab::refresh);
    connect(productTab_, &ProductTab::productsChanged, inventoryTab_, &InventoryTab::refresh);
    connect(inventoryTab_, &InventoryTab::inventoryChanged, productTab_, &ProductTab::refresh);

    connect(tabs_, &QTabWidget::currentChanged, this, &ManagerScreen::onSubTabChanged);

    mainLayout->addWidget(tabs_);
}

void ManagerScreen::refreshAll() {
    if (categoryTab_) categoryTab_->refresh();
    if (productTab_) productTab_->refresh();
    if (inventoryTab_) inventoryTab_->refresh();
    if (seasonTab_) seasonTab_->refresh();
    if (reportsTab_) reportsTab_->refresh();
}

void ManagerScreen::onSubTabChanged(int index) {
    switch (index) {
    case 0: if (categoryTab_) categoryTab_->refresh(); break;
    case 1: if (productTab_) productTab_->refresh(); break;
    case 2: if (inventoryTab_) inventoryTab_->refresh(); break;
    case 3: if (seasonTab_) seasonTab_->refresh(); break;
    case 4: if (reportsTab_) reportsTab_->refresh(); break;
    default: break;
    }
}

void ManagerScreen::showFrame(const Image& /*frame*/) {
    // Camera feed hook for barcode capture if needed
}

void ManagerScreen::handleCode(const std::string& /*code*/) {
    // Barcode scanned hook
}