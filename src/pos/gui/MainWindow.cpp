// MainWindow.cpp                                      Owner: Person 1
// Primary application window orchestrating Cashier, Manager, and Scanner interfaces.

#include "pos/gui/MainWindow.hpp"

#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPolygonF>
#include <QPushButton>
#include <QStatusBar>
#include <QTabWidget>
#include <QVBoxLayout>

#include "barcode/error.hpp"
#include "pos/common/PosError.hpp"
#include "pos/gui/ManagerApprovalDialog.hpp"
#include "pos/gui/ManagerScreen.hpp"
#include "pos/gui/OrderScreen.hpp"
#include "pos/gui/ScanScreen.hpp"
#include "pos/scanning/IBarcodeReader.hpp"
#include "pos/scanning/ScanController.hpp"
#include "pos/services/CategoryService.hpp"
#include "pos/services/CheckoutService.hpp"
#include "pos/services/OrderService.hpp"
#include "pos/services/ProductService.hpp"
#include "pos/services/ReportService.hpp"
#include "pos/services/SeasonService.hpp"

MainWindow::MainWindow(UserRole role,
    IFrameSource& source,
    ScanController& controller,
    IBarcodeReader& reader,
    ProductService& products,
    OrderService& orders,
    CheckoutService& checkout,
    CategoryService& categories,
    SeasonService& seasons,
    ReportService& reports,
    const std::string& receiptsDir,
    const std::string& shopName,
    const std::string& managerPin,
    QWidget* parent)
    : QMainWindow(parent),
    currentRole_(role),
    source_(source),
    controller_(controller),
    managerPin_(managerPin) {

    // ---------------- Header Bar ----------------
    auto* headerWidget = new QWidget(this);
    headerWidget->setObjectName("headerWidget");
    headerWidget->setFixedHeight(55);

    auto* brandLogo = new QLabel("?? <b>" + QString::fromStdString(shopName) + "</b> | Enterprise POS", headerWidget);
    brandLogo->setStyleSheet("font-size: 16px; color: #0F172A; font-weight: bold;");

    roleBadge_ = new QLabel(headerWidget);
    updateRoleDisplay();

    auto* switchRoleBtn = new QPushButton("Switch User / Role", headerWidget);
    switchRoleBtn->setCursor(Qt::PointingHandCursor);
    switchRoleBtn->setStyleSheet(
        "QPushButton { background-color: #E2E8F0; color: #1E293B; font-weight: 600; padding: 6px 14px; "
        "border-radius: 5px; border: 1px solid #CBD5E1; } "
        "QPushButton:hover { background-color: #CBD5E1; }");
    connect(switchRoleBtn, &QPushButton::clicked, this, &MainWindow::switchRole);

    auto* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(16, 0, 16, 0);
    headerLayout->addWidget(brandLogo);
    headerLayout->addStretch();
    headerLayout->addWidget(roleBadge_);
    headerLayout->addSpacing(12);
    headerLayout->addWidget(switchRoleBtn);

    // ---------------- Tab Screens ----------------
    tabs_ = new QTabWidget(this);
    tabs_->setObjectName("mainTabs");

    orderScreen_ = new OrderScreen(orders, products, checkout, receiptsDir, shopName, managerPin, this);
    tabs_->addTab(orderScreen_, "?? Cashier Terminal");

    managerScreen_ = new ManagerScreen(categories, products, seasons, reports, &reader, this);
    tabs_->addTab(managerScreen_, "?? Manager Workstation");

    scanScreen_ = new ScanScreen(products, reader, this);
    tabs_->addTab(scanScreen_, "?? Barcode Inspector");

    connect(tabs_, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    if (currentRole_ == UserRole::Manager) {
        tabs_->setCurrentIndex(1);
    }
    else {
        tabs_->setCurrentIndex(0);
    }

    // ---------------- Central Layout ----------------
    auto* centralContainer = new QWidget(this);
    auto* centralLayout = new QVBoxLayout(centralContainer);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);
    centralLayout->addWidget(headerWidget);
    centralLayout->addWidget(tabs_);

    setCentralWidget(centralContainer);
    setWindowTitle(QString::fromStdString(shopName));
    resize(1200, 780);
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
    }
    catch (const PosError& error) {
        statusBar()->showMessage(QString("Camera notice: ") + QString::fromUtf8(error.what()));
    }
    catch (const BarcodeError& error) {
        statusBar()->showMessage(QString("Camera notice: ") + QString::fromUtf8(error.what()));
    }
    catch (...) {
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
            roleBadge_->setText("?? Manager Mode");
            roleBadge_->setStyleSheet(
                "background-color: #DBEAFE; color: #1E40AF; font-weight: bold; padding: 4px 10px; "
                "border-radius: 12px; font-size: 12px; border: 1px solid #93C5FD;");
        }
        else {
            roleBadge_->setText("?? Cashier Mode");
            roleBadge_->setStyleSheet(
                "background-color: #D1FAE5; color: #065F46; font-weight: bold; padding: 4px 10px; "
                "border-radius: 12px; font-size: 12px; border: 1px solid #6EE7B7;");
        }
    }
}

void MainWindow::onTabChanged(int index) {
    if (index == 1 && currentRole_ != UserRole::Manager) {
        bool approved = ManagerApprovalDialog::requestApproval(
            this, managerPin_, "Enter Manager PIN to access the Manager Workstation:");

        if (approved) {
            currentRole_ = UserRole::Manager;
            updateRoleDisplay();
            managerScreen_->refreshAll();
            statusBar()->showMessage("Manager access authorized.", 3000);
        }
        else {
            statusBar()->showMessage("Access denied: Incorrect manager PIN.", 4000);
            tabs_->setCurrentIndex(0); // Return to Cashier
            return;
        }
    }
    else if (index == 1) {
        managerScreen_->refreshAll();
    }
}

void MainWindow::switchRole() {
    RoleDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        currentRole_ = dialog.role();
        updateRoleDisplay();
        if (currentRole_ == UserRole::Manager) {
            tabs_->setCurrentIndex(1);
            managerScreen_->refreshAll();
        }
        else {
            tabs_->setCurrentIndex(0);
        }
        statusBar()->showMessage(QString("Switched to %1 mode.").arg(currentRole_ == UserRole::Manager ? "Manager" : "Cashier"), 3000);
    }
}

void MainWindow::onFrame(const Image& frame) {
    QWidget* current = tabs_->currentWidget();
    if (current == orderScreen_) {
        orderScreen_->showFrame(frame);
    }
    else if (current == managerScreen_) {
        managerScreen_->showFrame(frame);
    }
    else if (current == scanScreen_) {
        scanScreen_->showFrame(frame);
    }
    controller_.onFrame(frame);
}

void MainWindow::onCode(const std::string& code) {
    QWidget* current = tabs_->currentWidget();
    if (current == orderScreen_) {
        orderScreen_->handleCode(code);
    }
    else if (current == managerScreen_) {
        managerScreen_->handleCode(code);
    }
    else if (current == scanScreen_) {
        scanScreen_->handleCode(code);
    }
}

void MainWindow::applyStyleSheet() {
    // Ensure arrow icons exist for spinboxes
    QDir().mkpath("data/icons");
    if (!QFile::exists("data/icons/spin_up.png")) {
        QPixmap upPix(16, 16);
        upPix.fill(Qt::transparent);
        QPainter p(&upPix);
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(QColor("#0F172A"));
        p.setPen(Qt::NoPen);
        QPolygonF poly;
        poly << QPointF(8, 4) << QPointF(13, 11) << QPointF(3, 11);
        p.drawPolygon(poly);
        p.end();
        upPix.save("data/icons/spin_up.png", "PNG");
    }
    if (!QFile::exists("data/icons/spin_down.png")) {
        QPixmap downPix(16, 16);
        downPix.fill(Qt::transparent);
        QPainter p(&downPix);
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(QColor("#0F172A"));
        p.setPen(Qt::NoPen);
        QPolygonF poly;
        poly << QPointF(3, 5) << QPointF(13, 5) << QPointF(8, 12);
        p.drawPolygon(poly);
        p.end();
        downPix.save("data/icons/spin_down.png", "PNG");
    }

    setStyleSheet(
        "QWidget { color: #0F172A; font-family: 'Segoe UI', Arial, sans-serif; }"
        "QMainWindow { background-color: #F8FAFC; }"
        "#headerWidget { background-color: #FFFFFF; border-bottom: 1px solid #CBD5E1; }"
        "QTabWidget::pane { border: none; background: #F8FAFC; }"
        "QTabBar::tab { background: #E2E8F0; color: #1E293B; padding: 10px 22px; font-weight: bold; font-size: 13px; "
        "border-top-left-radius: 6px; border-top-right-radius: 6px; margin-right: 4px; border: 1px solid #CBD5E1; border-bottom: none; }"
        "QTabBar::tab:selected { background: #FFFFFF; color: #0F172A; border-top: 3px solid #2563EB; font-weight: bold; }"
        "QTabBar::tab:hover:!selected { background: #CBD5E1; color: #000000; }"
        "QLabel { color: #0F172A; font-size: 13px; }"
        "QDialog { background-color: #F8FAFC; color: #0F172A; }"
        "QDialog QLabel { color: #0F172A; }"
        "QGroupBox { font-weight: bold; font-size: 14px; color: #0F172A; border: 1.5px solid #CBD5E1; "
        "border-radius: 8px; margin-top: 14px; padding-top: 16px; background-color: #FFFFFF; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 14px; padding: 0 8px; background-color: #FFFFFF; color: #0F172A; }"
        "QTableWidget { background-color: #FFFFFF; color: #0F172A; border: 1.5px solid #CBD5E1; border-radius: 6px; gridline-color: #E2E8F0; selection-background-color: #DBEAFE; selection-color: #1E3A8A; font-size: 13px; }"
        "QHeaderView::section { background-color: #F1F5F9; color: #0F172A; font-weight: bold; padding: 7px; border: none; border-bottom: 2px solid #CBD5E1; border-right: 1px solid #E2E8F0; font-size: 12px; }"
        "QLineEdit, QComboBox, QDoubleSpinBox, QSpinBox { padding: 6px 10px; border: 1.5px solid #94A3B8; border-radius: 5px; "
        "background-color: #FFFFFF; color: #0F172A; font-size: 13px; font-weight: 500; selection-background-color: #2563EB; selection-color: #FFFFFF; }"
        "QDoubleSpinBox, QSpinBox { padding-right: 26px; }"
        "QLineEdit:focus, QComboBox:focus, QDoubleSpinBox:focus, QSpinBox:focus { border: 2px solid #2563EB; color: #000000; }"
        "QLineEdit::placeholder { color: #64748B; font-weight: normal; }"
        "QDoubleSpinBox::up-button, QSpinBox::up-button { subcontrol-origin: border; subcontrol-position: top right; width: 22px; border-left: 1px solid #CBD5E1; border-bottom: 1px solid #CBD5E1; background: #F1F5F9; border-top-right-radius: 4px; }"
        "QDoubleSpinBox::up-button:hover, QSpinBox::up-button:hover { background: #E2E8F0; }"
        "QDoubleSpinBox::down-button, QSpinBox::down-button { subcontrol-origin: border; subcontrol-position: bottom right; width: 22px; border-left: 1px solid #CBD5E1; background: #F1F5F9; border-bottom-right-radius: 4px; }"
        "QDoubleSpinBox::down-button:hover, QSpinBox::down-button:hover { background: #E2E8F0; }"
        "QDoubleSpinBox::up-arrow, QSpinBox::up-arrow { image: url(data/icons/spin_up.png); width: 9px; height: 9px; }"
        "QDoubleSpinBox::down-arrow, QSpinBox::down-arrow { image: url(data/icons/spin_down.png); width: 9px; height: 9px; }"
        "QPushButton { padding: 7px 14px; border-radius: 5px; font-weight: 600; border: 1.5px solid #94A3B8; background-color: #F1F5F9; color: #0F172A; font-size: 13px; }"
        "QPushButton:hover { background-color: #E2E8F0; border-color: #64748B; color: #000000; }"
        "QPushButton:pressed { background-color: #CBD5E1; }"
        "QPushButton:disabled { background-color: #F8FAFC; color: #94A3B8; border-color: #E2E8F0; }"
        "QStatusBar { background-color: #FFFFFF; border-top: 1px solid #CBD5E1; color: #1E293B; font-size: 12px; font-weight: 500; }"
    );
}