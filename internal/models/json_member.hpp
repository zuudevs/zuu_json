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
    struct KeyStore {
        uint32_t index_;
        uint32_t length_;
    } key_{};
    JsonValue value_{};
};

} // namespace zuu::models