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

    const int marginX = 120;
    const int marginY = 100;
    const int pageWidth = pdfWriter.width();
    const int pageHeight = pdfWriter.height();
    const int contentWidth = pageWidth - (2 * marginX);

    // Standard receipt font with comfortable point size and painter-device metrics
    QFont monoFont("Courier New", 10);
    monoFont.setStyleHint(QFont::Monospace);
    painter.setFont(monoFont);

    // Initial baseline position
    int y = marginY + painter.fontMetrics().ascent();

    auto lines = ReceiptFormatter::lines(bill, shopName);
    for (const auto& line : lines) {
        QString qLine = QString::fromStdString(line).trimmed();

        // Prevent page overflow
        if (y + painter.fontMetrics().lineSpacing() + 30 > pageHeight - marginY) {
            pdfWriter.newPage();
            painter.setFont(monoFont);
            y = marginY + painter.fontMetrics().ascent();
        }

        // Horizontal divider rules
        if (qLine.startsWith("==") || qLine.startsWith("--")) {
            int lineY = y - (painter.fontMetrics().ascent() / 2);
            QPen pen(QColor("#64748b"), qLine.startsWith("==") ? 3 : 2, Qt::SolidLine);
            painter.setPen(pen);
            painter.drawLine(marginX, lineY, marginX + contentWidth, lineY);
            y += (painter.fontMetrics().lineSpacing() / 2) + 10;
            continue;
        }

        // Header store name or thank you message
        bool isStoreName = (line.find(shopName) != std::string::npos && !shopName.empty());
        bool isThankYou = (line.find("THANK YOU") != std::string::npos);
        bool isTotal = (line.find("TOTAL PAYABLE") != std::string::npos);

        if (isStoreName) {
            QFont titleFont("Courier New", 14, QFont::Bold);
            painter.setFont(titleFont);
            painter.setPen(QColor("#0f172a"));
            int textWidth = painter.fontMetrics().horizontalAdvance(qLine);
            int centerX = marginX + (contentWidth - textWidth) / 2;
            painter.drawText(centerX, y, qLine);
            painter.setFont(monoFont);
            y += painter.fontMetrics().lineSpacing() + 16;
            continue;
        }

        if (isThankYou) {
            QFont thankFont("Courier New", 12, QFont::Bold);
            painter.setFont(thankFont);
            painter.setPen(QColor("#1e40af"));
            int textWidth = painter.fontMetrics().horizontalAdvance(qLine);
            int centerX = marginX + (contentWidth - textWidth) / 2;
            painter.drawText(centerX, y, qLine);
            painter.setFont(monoFont);
            y += painter.fontMetrics().lineSpacing() + 12;
            continue;
        }

        if (isTotal) {
            QFont totalFont("Courier New", 11, QFont::Bold);
            painter.setFont(totalFont);
            painter.setPen(QColor("#0f172a"));
            painter.drawText(marginX, y, QString::fromStdString(line));
            painter.setFont(monoFont);
            y += painter.fontMetrics().lineSpacing() + 10;
            continue;
        }

        // Regular receipt line
        painter.setPen(QColor("#0f172a"));
        painter.drawText(marginX, y, QString::fromStdString(line));
        y += painter.fontMetrics().lineSpacing() + 8;
    }

    painter.end();
    return fullPath.toStdString();
}
