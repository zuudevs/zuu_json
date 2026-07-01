/**
 * @file general.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief General constants and C++23 compile-time SWAR generators
 * @version 1.0.0
 * @date 2026-06-10
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include <cstdint>

namespace zuu::constants {

constexpr uint8_t zero = 0;
constexpr uint8_t digit = 10;

constexpr uint8_t bit = 1;
constexpr uint8_t nibble = 4;
constexpr uint8_t byte = 8;
constexpr uint8_t word = 16;
constexpr uint8_t dword = 32;
constexpr uint8_t qword = 64;

constexpr uint8_t hex_alpha_max_val = 15;
constexpr uint8_t hex_alpha_min_val = digit;
constexpr uint8_t hex_digit_max_val = 9;
constexpr uint8_t hex_digit_min_val = zero;

constexpr unsigned char cache_line_size = 64;

constexpr uint8_t sso_tag = 0x80;
constexpr uint8_t sso_max_len = 7;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
constexpr uint32_t null_word = 0x6c6c756e;
constexpr uint32_t true_word = 0x65757274;
constexpr uint32_t fals_word = 0x736c6166;
#else
constexpr uint32_t null_word = 0x6e756c6c;
constexpr uint32_t true_word = 0x74727565;
constexpr uint32_t fals_word = 0x66616c73;
#endif

} // namespace zuu::constants