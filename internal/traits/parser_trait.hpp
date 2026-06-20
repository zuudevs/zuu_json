/**
 * @file parser_trait.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-17
 * 
 * @copyright Copyright (c) 2026
 */

#pragma once

namespace zuu::traits {

template <typename T> 
struct ParserTrait {
	static_assert(sizeof(T) == 0, "invalid size type specialized parser trait");
};

} // namespace zuu::traits