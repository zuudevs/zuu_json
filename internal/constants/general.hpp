/**
 * @file general.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-10
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

namespace zuu::constants {

constexpr unsigned char zero = 0;
constexpr unsigned char digit = 10;
constexpr unsigned char byte = 8;
constexpr unsigned char word = 16;
constexpr unsigned char dword = 32;

constexpr unsigned char uint8_max = 0xffui8;

constexpr unsigned char hex_alpha_max_val = 15;
constexpr unsigned char hex_alpha_min_val = digit;
constexpr unsigned char hex_digit_max_val = 9;
constexpr unsigned char hex_digit_min_val = zero;
constexpr unsigned char cache_line_size = 64;

constexpr unsigned long long swar_one_per_byte  = 0x0101010101010101ULL;
constexpr unsigned long long swar_high_bit_mask = 0x8080808080808080ULL;
constexpr unsigned long long swar_quote_bytes  = 0x2222222222222222ULL;
constexpr unsigned long long swar_escape_bytes = 0x5C5C5C5C5C5C5C5CULL;

#if  __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

constexpr unsigned null_word = 0x6c6c756e;
constexpr unsigned true_word = 0x65757274;
constexpr unsigned fals_word = 0x736c6166;

#else

constexpr unsigned null_word = 0x6e756c6c;
constexpr unsigned true_word = 0x74727565;
constexpr unsigned fals_word = 0x66616c73;

#endif

} // namespace zuu::constants