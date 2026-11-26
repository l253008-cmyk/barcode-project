// ReportTable.hpp                                     Owner: Person 4
// Shared tabular report structure for manager reporting.

#ifndef POS_REPORT_TABLE_HPP
#define POS_REPORT_TABLE_HPP

#include <string>
#include <vector>

struct ReportTable {
    std::string title;
    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> rows;
    std::vector<std::string> totalsRow; // Optional totals row (empty if not applicable)
};

#endif // POS_REPORT_TABLE_HPP