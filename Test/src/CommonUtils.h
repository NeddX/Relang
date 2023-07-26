#ifndef ALVM_TEST_COMMON_UTILITIES_H
#define ALVM_TEST_COMMON_UTILITIES_H

#include <string>
#include <vector>
#include <cstdint>

namespace std {
    std::vector<std::uint8_t> to_bytes(std::string&& str) {
        return std::vector<std::uint8_t>(str.begin(), str.end());
    }
    std::vector<std::uint8_t> to_bytes(const std::string& str) {
        return std::vector<std::uint8_t>(str.begin(), str.end());
    }
} // namespace std

#endif // ALVM_TEST_COMMON_UTILITIES_H
