/**
 * @file member_view.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-21
 * 
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "models/json_value.hpp"
#include <string_view>

namespace zuu::models {

class Json;
class Storage;
class Value; // Forward declaration

// ── Object Member View (Structured Binding Support) ──

struct MemberView {
    std::string_view key_;
    const Storage* storage_;
    JsonValue val_;

    // Konstruksi Value hanya dilakukan secara lazy saat dipanggil
    [[nodiscard]] Value value() const noexcept;
};

} // namespace zuu::models
