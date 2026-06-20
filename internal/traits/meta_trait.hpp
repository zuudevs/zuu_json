/**
 * @file meta_trait.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-20
 * 
 * @copyright Copyright (c) 2026
 */

namespace zuu::traits {

template <typename T>
struct MetaTrait {
    static_assert(sizeof(T) == 0, "invalid size type specialized meta trait");
};

} // namespace zuu::traits