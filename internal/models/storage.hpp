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

    [[nodiscard]] unsigned long long commitString(std::string_view value) noexcept;

    // Fast-path untuk unescaping
    [[nodiscard]] char* allocateStringBuffer(unsigned long long length) noexcept;

    // Fast-path untuk Parser (Bump Allocation)
    [[nodiscard]] unsigned long long getArrayOffset() const noexcept;
    void pushArrayElement(const JsonValue& val) noexcept;
    [[nodiscard]] unsigned long long sealArray(unsigned long long start_offset) noexcept;

    [[nodiscard]] unsigned long long getObjectOffset() const noexcept;
    void pushObjectMember(const JsonMember& member) noexcept;
    [[nodiscard]] unsigned long long sealObject(unsigned long long start_offset) noexcept;

    [[nodiscard]] JsonArray array(unsigned long long index) const noexcept;
    [[nodiscard]] JsonObject object(unsigned long long index) const noexcept;
    [[nodiscard]] std::string_view string(unsigned long long index) const noexcept;

  private:
    std::unique_ptr<std::byte[]> arena_;

    std::string_view* strings_{nullptr};
    unsigned strings_size_{0};

    JsonValue* array_elements_{nullptr};
    unsigned array_elements_size_{0};

    std::pair<unsigned, unsigned>* arrays_{nullptr};
    unsigned arrays_size_{0};

    JsonMember* object_elements_{nullptr};
    unsigned object_elements_size_{0};

    std::pair<unsigned, unsigned>* objects_{nullptr};
    unsigned objects_size_{0};

    // Buffer mentah untuk teks string yang membutuhkan Unescaping
    char* string_buffer_{nullptr};
    unsigned string_buffer_size_{0};

    JsonValue root_{};
    bool root_set_{false};
};

} // namespace zuu::models