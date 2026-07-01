/**
 * @file bit.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-07-01
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "constants/general.hpp"

namespace zuu::utils {

template <typename T>
[[nodiscard]] consteval T
    repeat_byte(unsigned char b) noexcept {
    T result = 0;
    for (auto i = 0; i < sizeof(T); ++i) {
        result |= (static_cast<T>(b) << (i * constants::byte));
    }
    return result;
}

} // namespace zuu::utils