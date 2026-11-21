// PdfReceiptPrinter.cpp                               Owner: Person 1
// Native PDF receipt generator using QPdfWriter and QPainter (Qt6::Gui).

#include "pos/gui/PdfReceiptPrinter.hpp"
#include "pos/services/ReceiptFormatter.hpp"
#include "pos/common/PosError.hpp"

#include <QDir>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPdfWriter>
#include <QPageSize>

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

    QFont font("Courier New", 9);
    font.setStyleHint(QFont::Monospace);
    painter.setFont(font);

    QFontMetrics fm(font);
    int lineHeight = fm.lineSpacing() + 6;
    int y = lineHeight * 2;
    int x = 60; // Left margin

    auto lines = ReceiptFormatter::lines(bill, shopName);
    for (const auto& line : lines) {
        painter.drawText(x, y, QString::fromStdString(line));
        y += lineHeight;
    }

    painter.end();
    return fullPath.toStdString();
}
