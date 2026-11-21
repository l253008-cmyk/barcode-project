#include "pos/storage/BillCodec.hpp"
#include "pos/common/PosError.hpp"
#include <cstdlib>
#include <iomanip>
#include <sstream>

namespace {

    const std::size_t kFixedTailTokens = 10;   // orderId ... status
    const std::size_t kMinTokens = 12;         // billId + at least one name token + 10 fixed
    const std::size_t kBillIdDigits = 9;
    const std::size_t kOrderIdDigits = 8;

    bool isDigits(const std::string& text, std::size_t length) {
        if (text.size() != length) {
            return false;
        }
        for (char c : text) {
            if (c < '0' || c > '9') {
                return false;
            }
        }
        return true;
    }

    // 2 becomes "2", 1.5 becomes "1.5" (no useless zeros).
    std::string formatQuantity(double quantity) {
        std::ostringstream out;
        out << std::fixed << std::setprecision(3) << quantity;
        std::string text = out.str();
        while (text.back() == '0') {
            text.pop_back();
        }
        if (text.back() == '.') {
            text.pop_back();
        }
        return text;
    }

    double parseQuantity(const std::string& text) {
        char* end = nullptr;
        double value = std::strtod(text.c_str(), &end);
        if (text.empty() || end != text.c_str() + text.size() || value <= 0) {
            throw PosError("Bad quantity in Bill.txt: '" + text + "'");
        }
        return value;
    }

    std::vector<std::string> splitWords(const std::string& text) {
        std::vector<std::string> words;
        std::string word;
        for (char c : text) {
            if (c == ' ') {
                if (!word.empty()) {
                    words.push_back(word);
                    word.clear();
                }
            }
            else {
                word += c;
            }
        }
        if (!word.empty()) {
            words.push_back(word);
        }
        return words;
    }

    // Reads one record into the whole-bill part (header) and one bill line.
    void parseRecord(const std::vector<std::string>& tokens, Bill& header, BillLine& line) {
        if (tokens.size() < kMinTokens) {
            throw PosError("Bill line has too few fields (need at least 12)");
        }

        const std::size_t tail = tokens.size() - kFixedTailTokens;   // index of the order id

        if (!isDigits(tokens[0], kBillIdDigits)) {
            throw PosError("Bill id must be 9 digits: '" + tokens[0] + "'");
        }
        if (!isDigits(tokens[tail], kOrderIdDigits)) {
            throw PosError("Order id must be 8 digits: '" + tokens[tail] + "'");
        }

        std::string name;
        for (std::size_t i = 1; i < tail; i++) {
            if (!name.empty()) {
                name += " ";
            }
            name += tokens[i];
        }
        if (name == "-") {
            name = "";   // "-" means that no customer name was given
        }

        header.id = tokens[0];
        header.customerName = name;
        header.orderId = tokens[tail];
        header.totalWithoutTax = Money::parse(tokens[tail + 5]);
        header.totalTax = Money::parse(tokens[tail + 6]);
        header.totalDiscount = Money::parse(tokens[tail + 7]);
        header.totalPayable = Money::parse(tokens[tail + 8]);
        header.status = tokens[tail + 9];

        line.acronym = tokens[tail + 1];
        line.unitPrice = Money::parse(tokens[tail + 2]);
        line.discount = Money::parse(tokens[tail + 3]);
        line.quantity = parseQuantity(tokens[tail + 4]);
    }

    bool sameHeader(const Bill& a, const Bill& b) {
        return a.id == b.id &&
            a.customerName == b.customerName &&
            a.orderId == b.orderId &&
            a.totalWithoutTax == b.totalWithoutTax &&
            a.totalTax == b.totalTax &&
            a.totalDiscount == b.totalDiscount &&
            a.totalPayable == b.totalPayable &&
            a.status == b.status;
    }

}  // namespace

std::vector<std::vector<std::string>> BillCodec::toRecords(const Bill& bill) {
    if (bill.lines.empty()) {
        throw PosError("A bill needs at least one line");
    }
    if (!isDigits(bill.id, kBillIdDigits)) {
        throw PosError("Bill id must be 9 digits: '" + bill.id + "'");
    }
    if (!isDigits(bill.orderId, kOrderIdDigits)) {
        throw PosError("Order id must be 8 digits: '" + bill.orderId + "'");
    }
    if (bill.status.empty() || bill.status.find(' ') != std::string::npos) {
        throw PosError("Bill status must be one word");
    }

    std::vector<std::string> nameWords = splitWords(bill.customerName);
    if (nameWords.empty()) {
        nameWords.push_back("-");   // keeps every column in place
    }

    std::vector<std::vector<std::string>> records;
    for (const BillLine& line : bill.lines) {
        if (line.acronym.empty() || line.acronym.find(' ') != std::string::npos) {
            throw PosError("Product acronym must be one word");
        }

        std::vector<std::string> record;
        record.push_back(bill.id);
        for (const std::string& word : nameWords) {
            record.push_back(word);
        }
        record.push_back(bill.orderId);
        record.push_back(line.acronym);
        record.push_back(line.unitPrice.toString());
        record.push_back(line.discount.toString());
        record.push_back(formatQuantity(line.quantity));
        record.push_back(bill.totalWithoutTax.toString());
        record.push_back(bill.totalTax.toString());
        record.push_back(bill.totalDiscount.toString());
        record.push_back(bill.totalPayable.toString());
        record.push_back(bill.status);
        records.push_back(record);
    }
    return records;
}

std::vector<Bill> BillCodec::fromRecords(const std::vector<std::vector<std::string>>& records) {
    std::vector<Bill> bills;

    for (const std::vector<std::string>& record : records) {
        Bill header;
        BillLine line;
        parseRecord(record, header, line);

        if (bills.empty() || bills.back().id != header.id) {
            bills.push_back(header);          // first line of a new bill
        }
        else if (!sameHeader(bills.back(), header)) {
            throw PosError("Lines of bill " + header.id + " do not have the same totals");
        }
        bills.back().lines.push_back(line);
    }
    return bills;
}