#include "pos/gui/MainWindow.hpp"

#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QPushButton>
#include <QStatusBar>
#include <QTabWidget>
#include <QVBoxLayout>

#include "barcode/error.hpp"
#include "pos/common/PosError.hpp"
#include "pos/gui/ManagerScreen.hpp"
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
      currentRole_(role),
      source_(source),
      controller_(controller) {

    // ---------------- Header Bar ----------------
    QWidget* headerWidget = new QWidget(this);
    headerWidget->setObjectName("headerWidget");
    headerWidget->setFixedHeight(55);

    QLabel* brandLogo = new QLabel("🏪 <b>Store POS System</b> | Enterprise", headerWidget);
    brandLogo->setStyleSheet("font-size: 16px; color: #0F172A; font-weight: bold;");

    roleBadge_ = new QLabel(headerWidget);
    updateRoleDisplay();

    QPushButton* switchRoleBtn = new QPushButton("Switch User / Role", headerWidget);
    switchRoleBtn->setCursor(Qt::PointingHandCursor);
    switchRoleBtn->setStyleSheet(
        "QPushButton { background-color: #E2E8F0; color: #1E293B; font-weight: 600; padding: 6px 14px; "
        "border-radius: 5px; border: 1px solid #CBD5E1; } "
        "QPushButton:hover { background-color: #CBD5E1; }");
    connect(switchRoleBtn, &QPushButton::clicked, this, &MainWindow::switchRole);

    QHBoxLayout* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(16, 0, 16, 0);
    headerLayout->addWidget(brandLogo);
    headerLayout->addStretch();
    headerLayout->addWidget(roleBadge_);
    headerLayout->addSpacing(12);
    headerLayout->addWidget(switchRoleBtn);

    // ---------------- Tab Screens ----------------
    tabs_ = new QTabWidget(this);
    tabs_->setObjectName("mainTabs");

    orderScreen_ = new OrderScreen(orders, products, checkout, this);
    tabs_->addTab(orderScreen_, "🛒 Cashier Terminal");

    managerScreen_ = new ManagerScreen(products, reader, this);
    tabs_->addTab(managerScreen_, "⚙️ Manager Workstation");

    scanScreen_ = new ScanScreen(products, reader, this);
    tabs_->addTab(scanScreen_, "🔍 Barcode Inspector");

    connect(tabs_, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    if (currentRole_ == UserRole::Manager) {
        tabs_->setCurrentIndex(1);
    } else {
        tabs_->setCurrentIndex(0);
    }

    // ---------------- Central Layout ----------------
    QWidget* centralContainer = new QWidget(this);
    QVBoxLayout* centralLayout = new QVBoxLayout(centralContainer);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);
    centralLayout->addWidget(headerWidget);
    centralLayout->addWidget(tabs_);

    setCentralWidget(centralContainer);
    setWindowTitle("Store POS System");
    resize(1150, 750);
    applyStyleSheet();

    // ---------------- Camera & Scanner Wiring ----------------
    source_.setFrameCallback([this](const Image& frame) { onFrame(frame); });
    controller_.setOnCode([this](const std::string& code) { onCode(code); });

    source_.setErrorCallback([this](const std::string& message) {
        statusBar()->showMessage(QString("Camera status: ") + QString::fromStdString(message) +
                                 ". (Use Open Image on Scan tab if camera is unavailable)");
    });

    try {
        source_.start();
        statusBar()->showMessage("Camera active. Ready for scanning.");
    } catch (const PosError& error) {
        statusBar()->showMessage(QString("Camera notice: ") + QString::fromUtf8(error.what()));
    } catch (const BarcodeError& error) {
        statusBar()->showMessage(QString("Camera notice: ") + QString::fromUtf8(error.what()));
    } catch (...) {
        statusBar()->showMessage("Camera offline. Barcodes can be typed manually or opened from file.");
    }
}

MainWindow::~MainWindow() {
    source_.stop();
    source_.setFrameCallback(nullptr);
    source_.setErrorCallback(nullptr);
    controller_.setOnCode(nullptr);
}

void MainWindow::updateRoleDisplay() {
    if (roleBadge_) {
        if (currentRole_ == UserRole::Manager) {
            roleBadge_->setText("🔵 Manager Mode");
            roleBadge_->setStyleSheet(
                "background-color: #DBEAFE; color: #1E40AF; font-weight: bold; padding: 4px 10px; "
                "border-radius: 12px; font-size: 12px; border: 1px solid #93C5FD;");
        } else {
            roleBadge_->setText("🟢 Cashier Mode");
            roleBadge_->setStyleSheet(
                "background-color: #D1FAE5; color: #065F46; font-weight: bold; padding: 4px 10px; "
                "border-radius: 12px; font-size: 12px; border: 1px solid #6EE7B7;");
        }
    }
}

void MainWindow::onTabChanged(int index) {
    // If switching to Manager tab from Cashier, authenticate
    if (index == 1 && currentRole_ != UserRole::Manager) {
        bool ok = false;
        QString pin = QInputDialog::getText(this, "Manager Authorization",
            "Enter Manager PIN (default: 1234):", QLineEdit::Password, "", &ok);

        if (ok && (pin.trimmed() == "1234" || pin.trimmed().isEmpty())) {
            currentRole_ = UserRole::Manager;
            updateRoleDisplay();
            managerScreen_->refreshInventory();
            statusBar()->showMessage("Manager access authorized.", 3000);
        } else {
            statusBar()->showMessage("Access denied: Incorrect manager PIN.", 4000);
            tabs_->setCurrentIndex(0); // Return to Cashier
            return;
        }
    } else if (index == 1) {
        managerScreen_->refreshInventory();
    }
}

void MainWindow::switchRole() {
    RoleDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        currentRole_ = dialog.role();
        updateRoleDisplay();
        if (currentRole_ == UserRole::Manager) {
            tabs_->setCurrentIndex(1);
            managerScreen_->refreshInventory();
        } else {
            tabs_->setCurrentIndex(0);
        }
        statusBar()->showMessage(QString("Switched to %1 mode.").arg(currentRole_ == UserRole::Manager ? "Manager" : "Cashier"), 3000);
    }
}

void MainWindow::onFrame(const Image& frame) {
    QWidget* current = tabs_->currentWidget();
    if (current == orderScreen_) {
        orderScreen_->showFrame(frame);
    } else if (current == managerScreen_) {
        managerScreen_->showFrame(frame);
    } else if (current == scanScreen_) {
        scanScreen_->showFrame(frame);
    }
    controller_.onFrame(frame);
}

void MainWindow::onCode(const std::string& code) {
    QWidget* current = tabs_->currentWidget();
    if (current == orderScreen_) {
        orderScreen_->handleCode(code);
    } else if (current == managerScreen_) {
        managerScreen_->handleCode(code);
    } else if (current == scanScreen_) {
        scanScreen_->handleCode(code);
    }
}

void MainWindow::applyStyleSheet() {
    setStyleSheet(
        "QMainWindow { background-color: #F8FAFC; }"
        "#headerWidget { background-color: #FFFFFF; border-bottom: 1px solid #E2E8F0; }"
        "QTabWidget::pane { border: none; background: #F8FAFC; }"
        "QTabBar::tab { background: #E2E8F0; color: #475569; padding: 10px 22px; font-weight: 600; font-size: 13px; "
        "border-top-left-radius: 6px; border-top-right-radius: 6px; margin-right: 4px; }"
        "QTabBar::tab:selected { background: #FFFFFF; color: #0F172A; border-bottom: 2px solid #2563EB; }"
        "QTabBar::tab:hover:!selected { background: #CBD5E1; }"
        "QGroupBox { font-weight: bold; font-size: 14px; color: #1E293B; border: 1px solid #CBD5E1; "
        "border-radius: 8px; margin-top: 14px; padding-top: 14px; background-color: #FFFFFF; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 14px; padding: 0 6px; background-color: #FFFFFF; }"
        "QTableWidget { background-color: #FFFFFF; border: 1px solid #CBD5E1; border-radius: 6px; gridline-color: #F1F5F9; }"
        "QHeaderView::section { background-color: #F1F5F9; color: #334155; font-weight: bold; padding: 6px; border: none; "
        "border-bottom: 1px solid #CBD5E1; }"
        "QLineEdit, QComboBox, QDoubleSpinBox, QSpinBox { padding: 6px 8px; border: 1px solid #CBD5E1; border-radius: 5px; "
        "background-color: #FFFFFF; font-size: 13px; }"
        "QLineEdit:focus, QComboBox:focus, QDoubleSpinBox:focus, QSpinBox:focus { border: 2px solid #3B82F6; }"
        "QPushButton { padding: 7px 14px; border-radius: 5px; font-weight: 600; border: 1px solid #CBD5E1; background-color: #F1F5F9; color: #1E293B; }"
        "QPushButton:hover { background-color: #E2E8F0; }"
        "QStatusBar { background-color: #FFFFFF; border-top: 1px solid #E2E8F0; color: #475569; font-size: 12px; }"
    );
}
