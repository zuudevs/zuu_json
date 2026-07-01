/**
 * @file member.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-21
 * 
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "allocators/storage.hpp"
#include "models/json_value.hpp"
#include <string_view>

namespace zuu::models {

class Json;
class Storage;
class Value;

} // namespace zuu::models

namespace zuu::models::views {

struct Member {
    std::string_view key_;
    const allocators::Storage* storage_;
    JsonValue val_;

    [[nodiscard]] Value value() const noexcept;
};

} // namespace zuu::models::views
