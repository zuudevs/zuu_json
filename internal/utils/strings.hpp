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
#include <cstdint>
#include <string>

namespace zuu::utils {

[[nodiscard]] inline constexpr bool
    is_numeric(char c) noexcept {
    return static_cast<uint8_t>(c - '0') < constants::digit;
}

[[nodiscard]] inline constexpr bool
    is_alphabet(char c) noexcept {
    return static_cast<uint8_t>((c | 0x20) - 'a') <= 25;
}

[[nodiscard]] inline constexpr bool
    is_whitespace(char c) noexcept {
    bool is_valid = (c >= '0' && c <= '9') || ((c | 0x20) >= 'a' && (c | 0x20) <= 'f');

    if (!is_valid) [[unlikely]] {
        return -1;
    }
    return (c & constants::hex_alpha_max_val) + (c >> 6) * constants::hex_digit_max_val;
}
[[nodiscard]] inline constexpr bool
    is_control_character(char c) noexcept {
    return static_cast<unsigned char>(c) < 0x20;
}
inline void
    encode_utf8(uint32_t cp, std::string& out) noexcept {
    if (cp <= 0x7F) {
        out += static_cast<char>(cp);
    } else if (cp <= 0x7FF) {
        char buf[2] = {static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)),
                       static_cast<char>(0x80 | (cp & 0x3F))};
        out.append(buf, 2);
    } else if (cp <= 0xFFFF) {
        char buf[3] = {static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)),
                       static_cast<char>(0x80 | ((cp >> 6) & 0x3F)),
                       static_cast<char>(0x80 | (cp & 0x3F))};
        out.append(buf, 3);
    } else if (cp <= 0x10FFFF) {
        char buf[4] = {static_cast<char>(0xF0 | ((cp >> 18) & 0x07)),
                       static_cast<char>(0x80 | ((cp >> 12) & 0x3F)),
                       static_cast<char>(0x80 | ((cp >> 6) & 0x3F)),
                       static_cast<char>(0x80 | (cp & 0x3F))};
        out.append(buf, 4);
    }
}

} // namespace zuu::utils