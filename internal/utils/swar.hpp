/**
 * @file swar.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief SIMD Within A Register utilities
 * @version 0.1.0
 * @date 2026-06-20
 * 
 * @copyright Copyright (c) 2026
 */

#pragma once

#include <bit>
#include "constants/swar.hpp"

namespace zuu::utils {

static_assert(std::endian::native == std::endian::little, "SWAR implementation currently requires Little Endian architecture");

/**
 * @brief Mencari lokasi byte nol dalam block T (umumnya uint64_t).
 * @return T Masker bit (bukan bool), di mana bit dengan posisi byte nol akan memiliki MSB = 1.
 */
template <typename T>
[[nodiscard]] inline constexpr T find_zero_byte_mask(T v) noexcept {
    return (v - constants::swar8_one) & ~v & constants::swar8_msb;
}


} // namespace zuu::utils