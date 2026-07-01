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

#include "utils/bit.hpp"

namespace zuu::constants {

alignas(8) constexpr auto swar8_zero = utils::repeat_byte<uint64_t>('0');
alignas(8) constexpr auto swar8_one = utils::repeat_byte<uint64_t>(0x01);
alignas(8) constexpr auto swar8_msb = utils::repeat_byte<uint64_t>(0x80);
alignas(8) constexpr auto swar8_digit_bias = utils::repeat_byte<uint64_t>(0x76);

alignas(8) constexpr auto swar8_dqt = utils::repeat_byte<uint64_t>('\"');
alignas(8) constexpr auto swar8_esc = utils::repeat_byte<uint64_t>('\\');
alignas(8) constexpr auto swar8_usc = utils::repeat_byte<uint64_t>('_');

alignas(8) constexpr auto swar8_ht = utils::repeat_byte<uint64_t>('\t');
alignas(8) constexpr auto swar8_lf = utils::repeat_byte<uint64_t>('\n');
alignas(8) constexpr auto swar8_vt = utils::repeat_byte<uint64_t>('\v');
alignas(8) constexpr auto swar8_ff = utils::repeat_byte<uint64_t>('\f');
alignas(8) constexpr auto swar8_cr = utils::repeat_byte<uint64_t>('\r');
alignas(8) constexpr auto swar8_sp = utils::repeat_byte<uint64_t>(' ');

alignas(8) constexpr auto swar8_lcb = utils::repeat_byte<uint64_t>('{');
alignas(8) constexpr auto swar8_rcb = utils::repeat_byte<uint64_t>('}');
alignas(8) constexpr auto swar8_lsb = utils::repeat_byte<uint64_t>('[');
alignas(8) constexpr auto swar8_rsb = utils::repeat_byte<uint64_t>(']');
alignas(8) constexpr auto swar8_com = utils::repeat_byte<uint64_t>(',');

} // namespace zuu::constants