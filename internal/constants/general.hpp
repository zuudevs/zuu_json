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

template <typename T>
[[nodiscard]] consteval T repeat_byte(uint8_t b) noexcept {
    T result = 0;
    for (auto i = 0; i < sizeof(T); ++i) {
        result |= (static_cast<T>(b) << (i * byte));
    }
    return result;
}

constexpr auto swar8_doublequote = repeat_byte<uint64_t>('\"');
constexpr auto swar8_escape      = repeat_byte<uint64_t>('\\');
constexpr auto swar8_underscore  = repeat_byte<uint64_t>('_');

constexpr auto swar8_ht          = repeat_byte<uint64_t>('\t');
constexpr auto swar8_lf          = repeat_byte<uint64_t>('\n');
constexpr auto swar8_vt          = repeat_byte<uint64_t>('\v');
constexpr auto swar8_ff          = repeat_byte<uint64_t>('\f');
constexpr auto swar8_cr          = repeat_byte<uint64_t>('\r');
constexpr auto swar8_sp          = repeat_byte<uint64_t>(' ');

constexpr auto swar8_zero        = repeat_byte<uint64_t>('0');
constexpr auto swar8_one         = repeat_byte<uint64_t>(0x01);
constexpr auto swar8_msb         = repeat_byte<uint64_t>(0x80);

constexpr auto swar8_digit_bias  = repeat_byte<uint64_t>(0x76);

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