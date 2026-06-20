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

#include <cstdint>
#include "models/json_value.hpp"

namespace zuu::models {

struct JsonMember {
	union KeyStore {
        uint64_t index_;
        struct {
            char chars_[sizeof(uint64_t) - 1];
            uint8_t length_tag_;
        } sso_;
    } key_{};
    JsonValue value_{};
};

} // namespace zuu::models