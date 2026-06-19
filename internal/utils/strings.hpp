/**
 * @file strings.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-01
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "constants/general.hpp"
#include <string>

namespace zuu::utils {

[[nodiscard]] inline constexpr bool is_numeric(char c) noexcept {
    return static_cast<unsigned char>(c - '0') < constants::digit;
}

[[nodiscard]] inline constexpr bool is_alphabet(char c) noexcept {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

[[nodiscard]] inline constexpr bool is_whitespace(char c) noexcept {
    return c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == ' ';
}

[[nodiscard]] inline constexpr int hex_to_int(char c) noexcept {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + constants::digit;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + constants::digit;
    return -1;
}

[[nodiscard]] inline constexpr unsigned long long build_string_prefix(std::string_view str) noexcept {
    unsigned long long len = str.size() > constants::uint8_max ? constants::uint8_max : str.size();
    unsigned long long prefix = (len << 24);
    if (str.size() > 0) prefix |= (static_cast<unsigned long long>(static_cast<unsigned char>(str[0])) << constants::word);
    if (str.size() > 1) prefix |= (static_cast<unsigned long long>(static_cast<unsigned char>(str[1])) << constants::byte);
    if (str.size() > 2) prefix |= (static_cast<unsigned long long>(static_cast<unsigned char>(str[2])));
    return prefix;
}

inline void encode_utf8(unsigned cp, std::string& out) noexcept {
    if (cp <= 0x7F) {
        // 1-byte (ASCII)
        out.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FF) {
        // 2-byte
        out.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        // 3-byte
        out.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0x10FFFF) {
        // 4-byte (Biasanya Emoji)
        out.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

} // namespace zuu::utils