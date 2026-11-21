#ifndef POS_COMMON_STRINGUTILS_HPP
#define POS_COMMON_STRINGUTILS_HPP

#include <string>

#include "pos/common/List.hpp"

namespace pos {

// Removes spaces, tabs and line endings from both ends.
std::string trim(const std::string& text);

// Splits on one character. Empty pieces are dropped, so "a  b" gives a and b.
List<std::string> split(const std::string& text, char separator = ' ');

// Puts the parts together again with the separator between them.
std::string join(const List<std::string>& parts, const std::string& separator = " ");

// Joins parts[from] up to but not including parts[to]. Used by the codecs to
// rebuild a name from the middle tokens of a line.
std::string joinRange(const List<std::string>& parts,
                      List<std::string>::size_type from,
                      List<std::string>::size_type to,
                      const std::string& separator = " ");

// True when the text is not empty and every character is 0-9.
bool isDigits(const std::string& text);

// Adds fill characters on the left until the text is width long.
std::string padLeft(const std::string& text, std::string::size_type width, char fill = '0');

// Removes a trailing '#' (and the space before it, if any) from a record line.
std::string stripEndMarker(const std::string& line);

// Strict number readers. They return false instead of throwing, so each caller
// can raise its own message. No exponent, no spaces, a dot for decimals.
bool parseInt(const std::string& text, long long& out);
bool parseDouble(const std::string& text, double& out);

// Quantity as written in the data files: 2, 1.5, 25.5 (at most 3 decimals,
// no trailing zeros, always a dot).
std::string formatQuantity(double value);

}  // namespace pos

#endif
