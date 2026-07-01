/**
 * @file depth_guard.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-21
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include <cstdint>

namespace zuu::models {

struct DepthGuard {
    uint32_t& depth_;
    inline explicit DepthGuard(uint32_t& d) noexcept
        : depth_(d) {
        ++depth_;
    }
    inline ~DepthGuard() noexcept {
        --depth_;
    }
};

} // namespace zuu::models