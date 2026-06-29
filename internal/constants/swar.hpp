/**
 * @file swar.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-29
 * 
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "constants/general.hpp"

namespace zuu::constants {

template <typename T>
[[nodiscard]] consteval T repeat_byte(uint8_t b) noexcept {
    T result = 0;
    for (auto i = 0; i < sizeof(T); ++i) {
        result |= (static_cast<T>(b) << (i * byte));
    }
    return result;
}

alignas(8) constexpr auto swar8_zero        = repeat_byte<uint64_t>('0');
alignas(8) constexpr auto swar8_one         = repeat_byte<uint64_t>(0x01);
alignas(8) constexpr auto swar8_msb         = repeat_byte<uint64_t>(0x80);
alignas(8) constexpr auto swar8_digit_bias  = repeat_byte<uint64_t>(0x76);

alignas(8) constexpr auto swar8_dqt         = repeat_byte<uint64_t>('\"');
alignas(8) constexpr auto swar8_esc         = repeat_byte<uint64_t>('\\');
alignas(8) constexpr auto swar8_usc         = repeat_byte<uint64_t>('_');

alignas(8) constexpr auto swar8_ht          = repeat_byte<uint64_t>('\t');
alignas(8) constexpr auto swar8_lf          = repeat_byte<uint64_t>('\n');
alignas(8) constexpr auto swar8_vt          = repeat_byte<uint64_t>('\v');
alignas(8) constexpr auto swar8_ff          = repeat_byte<uint64_t>('\f');
alignas(8) constexpr auto swar8_cr          = repeat_byte<uint64_t>('\r');
alignas(8) constexpr auto swar8_sp          = repeat_byte<uint64_t>(' ');

alignas(8) constexpr auto swar8_lcb         = repeat_byte<uint64_t>('{');
alignas(8) constexpr auto swar8_rcb         = repeat_byte<uint64_t>('}');
alignas(8) constexpr auto swar8_lsb         = repeat_byte<uint64_t>('[');
alignas(8) constexpr auto swar8_rsb         = repeat_byte<uint64_t>(']');
alignas(8) constexpr auto swar8_com         = repeat_byte<uint64_t>(',');

} // namespace zuu::constants