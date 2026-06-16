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

namespace zuu::models {

struct JsonMember {
    size_t key_index_{};
    JsonValue value_{};
};

} // namespace zuu::models