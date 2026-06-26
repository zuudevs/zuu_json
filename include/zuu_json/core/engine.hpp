/**
 * @file method.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-26
 * 
 * @copyright Copyright (c) 2026
 */

#pragma once

namespace zuu::core {

enum class TokenizerEngine : unsigned char {
	Swar,
	Avx2,
};

} // namespace zuu::core