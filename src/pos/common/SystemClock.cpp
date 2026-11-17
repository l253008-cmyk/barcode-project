#include "pos/common/SystemClock.hpp"

#include "pos/common/StringUtils.hpp"

// Reading the clock is not possible with <iostream> and <string> alone, so
// this is one of the two .cpp files (the other is TextRecordFile.cpp) that
// reach for another standard header. Nothing from them leaks into a header.
#include <chrono>
#include <ctime>

namespace pos {

namespace {

std::tm localTimeNow() {
    std::time_t seconds = std::time(nullptr);
    std::tm result = std::tm();
#ifdef _WIN32
    localtime_s(&result, &seconds);
#else
    localtime_r(&seconds, &result);
#endif
    return result;
}

std::string twoDigits(int value) { return padLeft(std::to_string(value), 2, '0'); }

}  // namespace

std::string SystemClock::today() const {
    std::tm t = localTimeNow();
    return padLeft(std::to_string(t.tm_year + 1900), 4, '0') + "-" +
           twoDigits(t.tm_mon + 1) + "-" + twoDigits(t.tm_mday);
}

std::string SystemClock::now() const {
    std::tm t = localTimeNow();
    return twoDigits(t.tm_hour) + ":" + twoDigits(t.tm_min) + ":" + twoDigits(t.tm_sec);
}

long long SystemClock::millis() const {
    auto since = std::chrono::steady_clock::now().time_since_epoch();
    return static_cast<long long>(
        std::chrono::duration_cast<std::chrono::milliseconds>(since).count());
}

}  // namespace pos
