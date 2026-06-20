/**
 * @file swar.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-20
 * 
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "constants/general.hpp"

namespace zuu::utils {

[[nodiscard]] inline constexpr bool 
find_zero_byte_mask(unsigned short v) noexcept {
	return (v - constants::swar2_ones) & ~v & constants::swar2_msb;
}

[[nodiscard]] inline constexpr bool 
find_zero_byte_mask(unsigned v) noexcept {
	return (v - constants::swar4_ones) & ~v & constants::swar4_msb;
}

[[nodiscard]] inline constexpr bool 
find_zero_byte_mask(unsigned long long v) noexcept {
	return (v - constants::swar8_ones) & ~v & constants::swar8_msb;
}


} // namespace zuu::utils