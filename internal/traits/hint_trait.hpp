/**
 * @file hint_trait.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-15
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

namespace zuu::traits {

template <typename>
struct HintTrait {
	static_assert(sizeof(HintTrait) == 0, "invalid size hint trait");
};

} // namespace zuu::traits