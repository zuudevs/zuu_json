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

template <typename T>
struct LookupTrait {
    static_assert(sizeof(T) == 0, "invalid size type specialized lookup trait");
};

} // namespace zuu::traits