/**
 * @file lookup_trait.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-16
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

namespace zuu::traits {

template <typename>
struct LookupTrait {
    static_assert(sizeof(LookupTrait) == 0, "invalid size lookup trait");
};

} // namespace zuu::traits