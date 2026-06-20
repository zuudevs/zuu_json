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


constexpr unsigned char bit = 1;
constexpr unsigned char nibble = 4;
constexpr unsigned char byte = 8;
constexpr unsigned char word = 16;
constexpr unsigned char dword = 32;
constexpr unsigned char qword = 64;

constexpr unsigned char hex_alpha_max_val = 15;
constexpr unsigned char hex_alpha_min_val = digit;
constexpr unsigned char hex_digit_max_val = 9;
constexpr unsigned char hex_digit_min_val = zero;

#define CREATE_SWAR_BYTES(ch, tag) \
	constexpr unsigned swar2_##tag = ((unsigned short)ch << 8) | (unsigned short)ch; \
	constexpr unsigned swar4_##tag = ((unsigned)swar2_##tag << 16) | (unsigned)swar2_##tag; \
	constexpr unsigned swar8_##tag = ((unsigned long long)swar4_##tag << 32) | (unsigned long long)swar4_##tag;

CREATE_SWAR_BYTES(0x01, ones);
CREATE_SWAR_BYTES(0x80, msb);
CREATE_SWAR_BYTES('\t', htab);
CREATE_SWAR_BYTES('\n', lf);
CREATE_SWAR_BYTES('\v', vtab);
CREATE_SWAR_BYTES('\f', ff);
CREATE_SWAR_BYTES('\r', cr);
CREATE_SWAR_BYTES(' ', space);
CREATE_SWAR_BYTES('\"', dquote);
CREATE_SWAR_BYTES('\\', escape);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	constexpr unsigned long long null_word = 0x6c6c756e;
	constexpr unsigned long long true_word = 0x65757274;
	constexpr unsigned long long false_word = 0x65736c6166;
#else
	constexpr unsigned long long null_word = 0x6e756c6c;
	constexpr unsigned long long true_word = 0x74727565;
	constexpr unsigned long long false_word = 0x66616c7365;
#endif

} // namespace zuu::constants