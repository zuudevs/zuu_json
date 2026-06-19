/**
 * @file swar.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-19
 * 
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "constants/general.hpp"

namespace zuu::utils {

[[nodiscard]] inline constexpr unsigned long long 
find_zero_byte_mask(unsigned long long v) noexcept {
	return (v - constants::swar_one_per_byte) & ~v & constants::swar_high_bit_mask;
}

[[nodiscard]] inline constexpr unsigned long long 
get_escape_mask(unsigned long long v) noexcept {
	return find_zero_byte_mask(v ^ constants::swar_escape_bytes);
}

[[nodiscard]] inline constexpr unsigned long long 
get_quote_or_escape_mask(unsigned long long v) noexcept {
	return find_zero_byte_mask(v ^ constants::swar_quote_bytes) | get_escape_mask(v);
}

} // namespace zuu::utils