/**
 * @file storage.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-06
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "models/json_member.hpp"
#include "models/token.hpp"
#include <memory>
#include <span>
#include <string_view>

namespace zuu::models {

class Storage {
  public:
    using Type = JsonValue::Type;
    using JsonArray = std::span<const JsonValue>;
    using JsonObject = std::span<const JsonMember>;

    Storage() noexcept = default;
    Storage(const Storage&) = delete;
    Storage(Storage&&) noexcept = default;
    Storage& operator=(const Storage&) = delete;
    Storage& operator=(Storage&&) noexcept = default;
    ~Storage() noexcept = default;

    Storage(models::Hint<Token> hint) noexcept;

    [[nodiscard]] bool hasRoot() const noexcept;
    void setRoot(JsonValue value) noexcept;
    [[nodiscard]] const JsonValue& root() const noexcept;

    [[nodiscard]] size_t commitString(std::string_view value) noexcept;

    // Fast-path untuk unescaping
    [[nodiscard]] char* allocateStringBuffer(size_t length) noexcept;

    // Fast-path untuk Parser (Bump Allocation)
    [[nodiscard]] size_t getArrayOffset() const noexcept;
    void pushArrayElement(const JsonValue& val) noexcept;
    [[nodiscard]] size_t sealArray(size_t start_offset) noexcept;

    [[nodiscard]] size_t getObjectOffset() const noexcept;
    void pushObjectMember(const JsonMember& member) noexcept;
    [[nodiscard]] size_t sealObject(size_t start_offset) noexcept;

    [[nodiscard]] JsonArray array(size_t index) const noexcept;
    [[nodiscard]] JsonObject object(size_t index) const noexcept;
    [[nodiscard]] std::string_view string(size_t index) const noexcept;

  private:
    std::unique_ptr<std::byte[]> arena_;

    std::string_view* strings_{nullptr};
    uint32_t strings_size_{0};

    JsonValue* array_elements_{nullptr};
    uint32_t array_elements_size_{0};

    std::pair<uint32_t, uint32_t>* arrays_{nullptr};
    uint32_t arrays_size_{0};

    JsonMember* object_elements_{nullptr};
    uint32_t object_elements_size_{0};

    std::pair<uint32_t, uint32_t>* objects_{nullptr};
    uint32_t objects_size_{0};

    // Buffer mentah untuk teks string yang membutuhkan Unescaping
    char* string_buffer_{nullptr};
    uint32_t string_buffer_size_{0};

    JsonValue root_{};
    bool root_set_{false};
};

} // namespace zuu::models