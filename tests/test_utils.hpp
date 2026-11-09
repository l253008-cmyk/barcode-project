#ifndef BARCODE_TEST_UTILS_HPP
#define BARCODE_TEST_UTILS_HPP
#include "barcode/error.hpp"
#include <iostream>
#include <string>
inline bool checkResult(const std::string& testName, const std::string& expected, const std::string& actual) {
    if (expected == actual) {
        std::cout << testName << " passed!" << std::endl;
        return true;
    }
    std::cout << testName << " failed!" << std::endl;
    std::cout << "Expected: " << expected << std::endl;
    std::cout << "Received: " << actual << std::endl;
    return false;
}
inline bool checkTrue(const std::string& testName, bool condition) {
    if (condition) {
        std::cout << testName << " passed!" << std::endl;
        return true;
    }
    std::cout << testName << " failed!" << std::endl;
    return false;
}
template <typename Function>
inline bool checkThrows(const std::string& testName, Function function) {
    try {
        function();
    }
    catch (const BarcodeError&) {
        std::cout << testName << " passed!" << std::endl;
        return true;
    }
    std::cout << testName << " failed!" << std::endl;
    return false;
}
#endif