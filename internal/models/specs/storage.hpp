/**
 * @file storage.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-07-01
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "models/json_member.hpp"

namespace zuu::models::specs {

struct Storage {
    const models::JsonMember* ptr{};
    uint32_t size{};
    bool is_sorted{};
};

} // namespace zuu::models::specs