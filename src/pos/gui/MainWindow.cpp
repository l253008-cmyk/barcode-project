#include "pos/gui/MainWindow.hpp"

#include <QStatusBar>
#include <QTabWidget>

#include "barcode/error.hpp"
#include "pos/common/PosError.hpp"
#include "pos/gui/OrderScreen.hpp"
#include "pos/gui/ScanScreen.hpp"
#include "pos/scanning/IBarcodeReader.hpp"
#include "pos/scanning/ScanController.hpp"
#include "pos/services/CheckoutService.hpp"
#include "pos/services/OrderService.hpp"
#include "pos/services/ProductService.hpp"

MainWindow::MainWindow(UserRole role, IFrameSource& source, ScanController& controller,
                       IBarcodeReader& reader, ProductService& products, OrderService& orders,
                       CheckoutService& checkout, QWidget* parent)
    : QMainWindow(parent),
      source_(source),
      controller_(controller),
      orderScreen_(nullptr),
      scanScreen_(nullptr) {
    tabs_ = new QTabWidget(this);

    if (role == UserRole::Cashier) {
        orderScreen_ = new OrderScreen(orders, products, checkout, this);
        tabs_->addTab(orderScreen_, "Order");
    }
    scanScreen_ = new ScanScreen(products, reader, this);
    tabs_->addTab(scanScreen_, "Scan");

    setCentralWidget(tabs_);
    setWindowTitle(role == UserRole::Cashier ? "Store POS - Cashier" : "Store POS - Manager");
    resize(1100, 700);

    source_.setFrameCallback([this](const Image& frame) { onFrame(frame); });
    controller_.setOnCode([this](const std::string& code) { onCode(code); });

    source_.setErrorCallback([this](const std::string& message) {
        statusBar()->showMessage(QString("Camera problem: ") + QString::fromStdString(message) +
                                 ". Use the Open Image button on the Scan tab.");
    });

    try {
        source_.start();
        statusBar()->showMessage("Camera started.");
    } catch (const PosError& error) {
        statusBar()->showMessage(QString("Camera problem: ") + QString::fromUtf8(error.what()) +
                                 ". Use the Open Image button on the Scan tab.");
    } catch (const BarcodeError& error) {
        statusBar()->showMessage(QString("Camera problem: ") + QString::fromUtf8(error.what()) +
                                 ". Use the Open Image button on the Scan tab.");
    } catch (...) {
        statusBar()->showMessage("Camera problem. Use the Open Image button on the Scan tab.");
    }
}

MainWindow::~MainWindow() {
    source_.stop();
    source_.setFrameCallback(nullptr);
    source_.setErrorCallback(nullptr);
    controller_.setOnCode(nullptr);
}

void MainWindow::onFrame(const Image& frame) {
    QWidget* current = tabs_->currentWidget();
    if (current == orderScreen_) {
        orderScreen_->showFrame(frame);
    } else if (current == scanScreen_) {
        scanScreen_->showFrame(frame);
    }
    controller_.onFrame(frame);
}

void MainWindow::onCode(const std::string& code) {
    QWidget* current = tabs_->currentWidget();
    if (current == orderScreen_) {
        orderScreen_->handleCode(code);
    } else if (current == scanScreen_) {
        scanScreen_->handleCode(code);
    }
}
