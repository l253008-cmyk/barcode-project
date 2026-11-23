// PdfReceiptPrinter.cpp                               Owner: Person 1
// Native PDF receipt generator using QPdfWriter and QPainter (Qt6::Gui).

#include "pos/gui/PdfReceiptPrinter.hpp"
#include "pos/services/ReceiptFormatter.hpp"
#include "pos/common/PosError.hpp"

#include <QColor>
#include <QDir>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPageSize>
#include <QPdfWriter>
#include <QPen>

std::string PdfReceiptPrinter::printReceipt(const Bill& bill,
                                            const std::string& receiptDir,
                                            const std::string& shopName) {
    QDir dir(QString::fromStdString(receiptDir));
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString filename = QString("bill_%1.pdf").arg(QString::fromStdString(bill.id));
    QString fullPath = dir.filePath(filename);

    QPdfWriter pdfWriter(fullPath);
    pdfWriter.setPageSize(QPageSize(QPageSize::A5));
    pdfWriter.setResolution(300);

    QPainter painter(&pdfWriter);
    if (!painter.isActive()) {
        throw PosError("Failed to initialize PDF painter for: " + fullPath.toStdString());
    }

    // A5 page geometry at 300 DPI: ~1748 x ~2480 pixels
    const int marginX = 140;
    const int pageWidth = 1748;
    const int contentWidth = pageWidth - (2 * marginX);

    // Standard receipt font with comfortable point size and generous line height
    QFont monoFont("Courier New", 13);
    monoFont.setStyleHint(QFont::Monospace);
    monoFont.setWeight(QFont::Medium);
    painter.setFont(monoFont);

    QFontMetrics fm(monoFont);
    const int lineHeight = static_cast<int>(fm.height() * 1.75); // Generous vertical spacing

    int y = 140; // Top margin

    auto lines = ReceiptFormatter::lines(bill, shopName);
    for (const auto& line : lines) {
        QString qLine = QString::fromStdString(line).trimmed();

        // If line is a separator like "==================" or "------------------"
        if (qLine.startsWith("==") || qLine.startsWith("--")) {
            y += 8;
            QPen pen(QColor("#64748b"), 3, Qt::SolidLine);
            painter.setPen(pen);
            painter.drawLine(marginX, y, marginX + contentWidth, y);
            y += 24;
            continue;
        }

        // Header store name or thank you message
        bool isStoreName = (line.find(shopName) != std::string::npos && !shopName.empty());
        bool isThankYou = (line.find("THANK YOU") != std::string::npos);
        bool isTotal = (line.find("TOTAL PAYABLE") != std::string::npos);

        if (isStoreName) {
            QFont titleFont("Courier New", 16, QFont::Bold);
            painter.setFont(titleFont);
            painter.setPen(QColor("#0f172a"));
            QFontMetrics tfm(titleFont);
            int textWidth = tfm.horizontalAdvance(qLine);
            int centerX = marginX + (contentWidth - textWidth) / 2;
            painter.drawText(centerX, y, qLine);
            painter.setFont(monoFont);
            y += static_cast<int>(lineHeight * 1.2);
            continue;
        }

        if (isThankYou) {
            QFont thankFont("Courier New", 14, QFont::Bold);
            painter.setFont(thankFont);
            painter.setPen(QColor("#1e40af"));
            QFontMetrics yfm(thankFont);
            int textWidth = yfm.horizontalAdvance(qLine);
            int centerX = marginX + (contentWidth - textWidth) / 2;
            painter.drawText(centerX, y, qLine);
            painter.setFont(monoFont);
            y += lineHeight;
            continue;
        }

        if (isTotal) {
            QFont totalFont("Courier New", 14, QFont::Bold);
            painter.setFont(totalFont);
            painter.setPen(QColor("#0f172a"));
            painter.drawText(marginX, y, QString::fromStdString(line));
            painter.setFont(monoFont);
            y += static_cast<int>(lineHeight * 1.15);
            continue;
        }

        // Regular line
        painter.setPen(QColor("#0f172a"));
        painter.drawText(marginX, y, QString::fromStdString(line));
        y += lineHeight;
    }

    painter.end();
    return fullPath.toStdString();
}
