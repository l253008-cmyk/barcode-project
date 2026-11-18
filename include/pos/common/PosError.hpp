#ifndef POS_COMMON_POSERROR_HPP
#define POS_COMMON_POSERROR_HPP

#include <string>

namespace pos {

// The one error type used by the POS code. It is kept apart from BarcodeError
// on purpose: an unreadable barcode and a damaged data file are different
// problems, and the GUI shows them differently.
class PosError {
public:
    explicit PosError(const std::string& message) : message_(message) {}

    const char* what() const noexcept { return message_.c_str(); }
    const std::string& message() const noexcept { return message_; }

private:
    std::string message_;
};

}  // namespace pos

#endif
