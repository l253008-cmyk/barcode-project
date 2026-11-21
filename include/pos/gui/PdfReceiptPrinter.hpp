// PdfReceiptPrinter.hpp                               Owner: Person 1
// Native PDF receipt generator using QPdfWriter and QPainter (Qt6::Gui).

#ifndef POS_PDF_RECEIPT_PRINTER_HPP
#define POS_PDF_RECEIPT_PRINTER_HPP

#include <string>
#include "pos/domain/Bill.hpp"

class PdfReceiptPrinter {
public:
    // Prints a PDF receipt for the bill to receiptDir/bill_<id>.pdf.
    // Returns the path of the generated PDF file.
    static std::string printReceipt(const Bill& bill,
                                    const std::string& receiptDir,
                                    const std::string& shopName = "Store POS System");
};

#endif // POS_PDF_RECEIPT_PRINTER_HPP
