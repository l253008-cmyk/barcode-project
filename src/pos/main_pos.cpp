// main_pos.cpp                                        Owner: Person 1
// Composition root and entry point for the Qt-based Point of Sale (POS) application.

#include <QApplication>

#include "pos/common/AppConfig.hpp"
#include "pos/common/SystemClock.hpp"
#include "pos/gui/MainWindow.hpp"
#include "pos/gui/RoleDialog.hpp"
#include "pos/scanning/Ean13Reader.hpp"
#include "pos/scanning/QtCameraSource.hpp"
#include "pos/scanning/ScanController.hpp"
#include "pos/services/BillingService.hpp"
#include "pos/services/CheckoutService.hpp"
#include "pos/services/IdGenerator.hpp"
#include "pos/services/OrderService.hpp"
#include "pos/services/ProductService.hpp"
#include "pos/services/StockPolicy.hpp"
#include "pos/storage/FileBillRepository.hpp"
#include "pos/storage/FileOrderDetailRepository.hpp"
#include "pos/storage/FileOrderRepository.hpp"
#include "pos/storage/FileProductRepository.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#F8FAFC"));
    palette.setColor(QPalette::WindowText, QColor("#0F172A"));
    palette.setColor(QPalette::Base, Qt::white);
    palette.setColor(QPalette::AlternateBase, QColor("#F1F5F9"));
    palette.setColor(QPalette::ToolTipBase, Qt::white);
    palette.setColor(QPalette::ToolTipText, QColor("#0F172A"));
    palette.setColor(QPalette::Text, QColor("#0F172A"));
    palette.setColor(QPalette::Button, QColor("#F1F5F9"));
    palette.setColor(QPalette::ButtonText, QColor("#0F172A"));
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Highlight, QColor("#2563EB"));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::PlaceholderText, QColor("#64748B"));
    app.setPalette(palette);

    // 1. Role chooser dialog
    RoleDialog roleDialog;
    if (roleDialog.exec() != QDialog::Accepted) {
        return 0;
    }

    // 2. Wiring domain & infrastructure
    pos::SystemClock clock;

    FileProductRepository productRepo(pos::config::productFile());
    pos::FileOrderRepository orderRepo(pos::config::orderFile());
    pos::FileOrderDetailRepository orderDetailRepo(pos::config::orderDetailFile());
    FileBillRepository billRepo(pos::config::billFile());

    pos::IdGenerator idGenerator(orderRepo, billRepo, clock);
    StockPolicy stockPolicy;
    ProductService productService(productRepo, stockPolicy);
    OrderService orderService(productRepo, productService, orderRepo, orderDetailRepo, idGenerator, clock);

    BillingService billingService([&idGenerator]() { return idGenerator.nextBillId(); });
    CheckoutService checkoutService(orderService, billingService, billRepo);

    // 3. Scanning & Camera
    QtCameraSource cameraSource;
    Ean13Reader barcodeReader;
    ScanController scanController(barcodeReader, clock);

    // 4. Main Window
    MainWindow mainWindow(roleDialog.role(), cameraSource, scanController, barcodeReader,
                          productService, orderService, checkoutService);
    mainWindow.show();

    return app.exec();
}
