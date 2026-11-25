// ManagerScreen.hpp                                   Owner: Person 1
// Comprehensive Manager Workstation hosting Categories, Products, Inventory, Seasons, and Reports.

#ifndef POS_MANAGER_SCREEN_HPP
#define POS_MANAGER_SCREEN_HPP

#include <QTabWidget>
#include <QWidget>

#include "barcode/image.hpp"
#include "pos/scanning/IBarcodeReader.hpp"
#include "pos/services/CategoryService.hpp"
#include "pos/services/ProductService.hpp"
#include "pos/services/ReportService.hpp"
#include "pos/services/SeasonService.hpp"

class CategoryTab;
class ProductTab;
class InventoryTab;
class SeasonTab;
class ReportsTab;

class ManagerScreen : public QWidget {
    Q_OBJECT

public:
    ManagerScreen(CategoryService& categories,
        ProductService& products,
        SeasonService& seasons,
        ReportService& reports,
        IBarcodeReader* reader = nullptr,
        QWidget* parent = nullptr);

    // Refresh all sub-tabs
    void refreshAll();

    // Camera feed hooks (for camera-based barcode scanning if needed)
    void showFrame(const Image& frame);
    void handleCode(const std::string& code);

private slots:
    void onSubTabChanged(int index);

private:
    void setupUi();

    CategoryService& categoryService_;
    ProductService& productService_;
    SeasonService& seasonService_;
    ReportService& reportService_;
    IBarcodeReader* reader_ = nullptr;

    QTabWidget* tabs_ = nullptr;
    CategoryTab* categoryTab_ = nullptr;
    ProductTab* productTab_ = nullptr;
    InventoryTab* inventoryTab_ = nullptr;
    SeasonTab* seasonTab_ = nullptr;
    ReportsTab* reportsTab_ = nullptr;
};

#endif // POS_MANAGER_SCREEN_HPP