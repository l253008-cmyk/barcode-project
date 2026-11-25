// DateUtils.cpp                                       Owner: Person 3
// Implementation of DateUtils calendar algorithms.

#include "pos/common/DateUtils.hpp"
#include "pos/common/PosError.hpp"

#include <cstdio>
#include <string>

namespace pos {
    namespace DateUtils {

        static bool isLeapYear(int y) {
            return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
        }

        static int daysInMonth(int y, int m) {
            if (m < 1 || m > 12) return 0;
            static const int kDays[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
            if (m == 2 && isLeapYear(y)) return 29;
            return kDays[m];
        }

        static bool parseDateParts(const std::string& s, int& y, int& m, int& d) {
            if (s.size() != 10 || s[4] != '-' || s[7] != '-') return false;
            for (size_t i = 0; i < s.size(); ++i) {
                if (i == 4 || i == 7) continue;
                if (s[i] < '0' || s[i] > '9') return false;
            }
            y = std::stoi(s.substr(0, 4));
            m = std::stoi(s.substr(5, 2));
            d = std::stoi(s.substr(8, 2));
            if (y < 1900 || y > 2100) return false;
            if (m < 1 || m > 12) return false;
            if (d < 1 || d > daysInMonth(y, m)) return false;
            return true;
        }

        // Convert date to Julian Day Number for exact calendar arithmetic
        static long long toJdn(int y, int m, int d) {
            int a = (14 - m) / 12;
            long long yr = y + 4800 - a;
            int mo = m + 12 * a - 3;
            return d + (153 * mo + 2) / 5 + 365 * yr + yr / 4 - yr / 100 + yr / 400 - 32045;
        }

        bool isValidDate(const std::string& dateStr) {
            int y = 0, m = 0, d = 0;
            return parseDateParts(dateStr, y, m, d);
        }

        int daysBetween(const std::string& dateA, const std::string& dateB) {
            int y1 = 0, m1 = 0, d1 = 0;
            int y2 = 0, m2 = 0, d2 = 0;
            if (!parseDateParts(dateA, y1, m1, d1)) {
                throw PosError("Invalid date format: '" + dateA + "', expected YYYY-MM-DD");
            }
            if (!parseDateParts(dateB, y2, m2, d2)) {
                throw PosError("Invalid date format: '" + dateB + "', expected YYYY-MM-DD");
            }
            return static_cast<int>(toJdn(y2, m2, d2) - toJdn(y1, m1, d1));
        }

        bool isDateInRange(const std::string& date, const std::string& start, const std::string& end) {
            if (!isValidDate(date) || !isValidDate(start) || !isValidDate(end)) {
                return false;
            }
            return date >= start && date <= end;
        }

    } // namespace DateUtils
} // namespace posS