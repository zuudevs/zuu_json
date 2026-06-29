/**
 * @file meta_trait.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-20
 * * @copyright Copyright (c) 2026
 */

#pragma once

#include <cstdint>

namespace zuu::models { 
	struct JsonMember; 
	class Storage; 
}

namespace zuu::traits {

template <typename T>
struct MetaTrait {
    static_assert(sizeof(T) == 0, "invalid size type specialized meta trait");
};

template <>
struct MetaTrait<models::Storage> {
    const models::JsonMember* ptr{};
    uint32_t size{};
    bool is_sorted{};
};

} // namespace zuu::traits