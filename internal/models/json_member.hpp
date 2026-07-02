/**
 * @file json_member.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-06
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "models/json_value.hpp"
#include <cstdint>

namespace zuu::models {

struct JsonMember {
    struct KeyStore {
        union {
            struct {
                uint32_t index_;
                uint32_t length_;
            } ref_;

            struct {
                char chars_[7];
                uint8_t tag_;
            } sso_;
        };
    } key_{};
    JsonValue value_{};
};

} // namespace zuu::models