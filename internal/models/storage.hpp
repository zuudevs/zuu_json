/**
 * @file storage.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-06
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "hint.hpp"
#include "models/json_member.hpp"
#include "models/token.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace zuu::models {

class Storage {
  public:
    using Type = JsonValue::Type;
    using JsonArray = std::vector<JsonValue>;
    using JsonObject = std::vector<JsonMember>;

    Storage() noexcept = default;
    Storage(Storage&&) noexcept = default;
    Storage& operator=(Storage&&) noexcept = default;
    Storage(const Storage&) noexcept = delete;
    Storage& operator=(const Storage&) noexcept = delete;
    ~Storage() noexcept = default;
    Storage(models::Hint<models::Token> hint) noexcept;

    [[nodiscard]] bool hasRoot() const noexcept;
    void setRoot(JsonValue value) noexcept;
    [[nodiscard]] const JsonValue& root() const noexcept;

    [[nodiscard]] size_t addString(std::string_view value) noexcept;
    [[nodiscard]] size_t addArray() noexcept;
    [[nodiscard]] size_t addObject() noexcept;

    [[nodiscard]] JsonArray& array(size_t index) noexcept;
    [[nodiscard]] const JsonArray& array(size_t index) const noexcept;
    [[nodiscard]] JsonObject& object(size_t index) noexcept;
    [[nodiscard]] const JsonObject& object(size_t index) const noexcept;
    [[nodiscard]] const std::string& string(size_t index) const noexcept;

  private:
    std::vector<std::string> strings_;
    std::vector<JsonObject> objects_;
    std::vector<JsonArray> arrays_;
    JsonValue root_{};
    bool root_set_{false};
};

} // namespace zuu::models