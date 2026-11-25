// DateUtils.hpp                                       Owner: Person 3
// Calendar utilities for POS date validation, comparison, and range checking.

#ifndef POS_DATE_UTILS_HPP
#define POS_DATE_UTILS_HPP

#include <string>

namespace pos {
	namespace DateUtils {

		// Checks whether text is a valid calendar date in YYYY-MM-DD format.
		bool isValidDate(const std::string& dateStr);

		// Returns (dateB - dateA) in days. Positive if dateB is after dateA.
		// Throws PosError if either date is invalid.
		int daysBetween(const std::string& dateA, const std::string& dateB);

		// Returns true if date is within [start, end] inclusive.
		bool isDateInRange(const std::string& date, const std::string& start, const std::string& end);

	} // namespace DateUtils
} // namespace pos

#endif // POS_DATE_UTILS_HPP