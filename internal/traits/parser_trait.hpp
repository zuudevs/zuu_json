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

template <typename> 
struct ParserTrait {
	static_assert(sizeof(ParserTrait) == 0, "invalid size parser trait");
};

} // namespace zuu::traits