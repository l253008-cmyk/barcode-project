#include "pos/common/StringUtils.hpp"

namespace pos {

namespace {
bool isDigitChar(char c) { return c >= '0' && c <= '9'; }
}  // namespace

std::string trim(const std::string& text) {
    const char* blanks = " \t\r\n";
    std::string::size_type first = text.find_first_not_of(blanks);
    if (first == std::string::npos) return std::string();
    std::string::size_type last = text.find_last_not_of(blanks);
    return text.substr(first, last - first + 1);
}

List<std::string> split(const std::string& text, char separator) {
    List<std::string> parts;
    std::string current;
    for (std::string::size_type i = 0; i < text.size(); ++i) {
        if (text[i] == separator) {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
        } else {
            current += text[i];
        }
    }
    if (!current.empty()) parts.push_back(current);
    return parts;
}

std::string join(const List<std::string>& parts, const std::string& separator) {
    return joinRange(parts, 0, parts.size(), separator);
}

std::string joinRange(const List<std::string>& parts,
                      List<std::string>::size_type from,
                      List<std::string>::size_type to,
                      const std::string& separator) {
    if (to > parts.size()) to = parts.size();
    std::string result;
    for (List<std::string>::size_type i = from; i < to; ++i) {
        if (i > from) result += separator;
        result += parts[i];
    }
    return result;
}

bool isDigits(const std::string& text) {
    if (text.empty()) return false;
    for (std::string::size_type i = 0; i < text.size(); ++i) {
        if (!isDigitChar(text[i])) return false;
    }
    return true;
}

std::string padLeft(const std::string& text, std::string::size_type width, char fill) {
    if (text.size() >= width) return text;
    return std::string(width - text.size(), fill) + text;
}

std::string stripEndMarker(const std::string& line) {
    std::string text = trim(line);
    if (!text.empty() && text[text.size() - 1] == '#') {
        text.erase(text.size() - 1);
        text = trim(text);
    }
    return text;
}

bool parseInt(const std::string& text, long long& out) {
    std::string::size_type i = 0;
    bool negative = false;
    if (i < text.size() && text[i] == '-') {
        negative = true;
        ++i;
    }
    std::string::size_type digits = text.size() - i;
    if (digits == 0 || digits > 18) return false;  // 18 digits always fit in long long

    long long value = 0;
    for (; i < text.size(); ++i) {
        if (!isDigitChar(text[i])) return false;
        value = value * 10 + (text[i] - '0');
    }
    out = negative ? -value : value;
    return true;
}

bool parseDouble(const std::string& text, double& out) {
    std::string::size_type i = 0;
    const std::string::size_type n = text.size();
    bool negative = false;
    if (i < n && text[i] == '-') {
        negative = true;
        ++i;
    }

    double value = 0.0;
    std::string::size_type intDigits = 0;
    while (i < n && isDigitChar(text[i])) {
        value = value * 10.0 + (text[i] - '0');
        ++i;
        ++intDigits;
    }
    if (intDigits == 0 || intDigits > 15) return false;

    if (i < n && text[i] == '.') {
        ++i;
        double fraction = 0.0;
        double scale = 1.0;
        std::string::size_type fractionDigits = 0;
        while (i < n && isDigitChar(text[i])) {
            fraction = fraction * 10.0 + (text[i] - '0');
            scale *= 10.0;
            ++i;
            ++fractionDigits;
        }
        if (fractionDigits == 0 || fractionDigits > 9) return false;
        value += fraction / scale;  // one division keeps 25.5 exactly 25.5
    }

    if (i != n) return false;
    out = negative ? -value : value;
    return true;
}

std::string formatQuantity(double value) {
    bool negative = value < 0.0;
    double magnitude = negative ? -value : value;

    long long thousandths = static_cast<long long>(magnitude * 1000.0 + 0.5);
    long long whole = thousandths / 1000;
    long long fraction = thousandths % 1000;

    std::string result = std::to_string(whole);
    if (fraction != 0) {
        std::string digits = padLeft(std::to_string(fraction), 3, '0');
        while (!digits.empty() && digits[digits.size() - 1] == '0') {
            digits.erase(digits.size() - 1);
        }
        result += "." + digits;
    }
    if (negative && thousandths != 0) result = "-" + result;
    return result;
}

}  // namespace pos
