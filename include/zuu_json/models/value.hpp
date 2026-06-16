/**
 * @file value.hpp
 * @author zuu::modelsdevs (zuu::modelsdevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-05
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "models/json_value.hpp"
#include "zuu_json/core/error.hpp"
#include <expected>
#include <string_view>

namespace zuu::models {

class Storage;
class Json;

/**
 * @brief A read-only view of a single JSON value inside a parsed document.
 *
 * Lightweight — holds a pointer back to the owning Storage plus the
 * value's type and raw data.  Remains valid as long as the originating
 * Json object is alive.
 *
 * Typed access:
 * @code
 *   auto doc  = Json::parse(R"({"x": 3.14})").value();
 *   auto pi   = doc["x"]->as_double().value();  // 3.14L
 * @endcode
 */
class Value {
  public:
    template <typename T>
    using Result = std::expected<T, core::JsonError>;
    using Type = models::JsonValue::Type;
    using Storage = Storage;
    using JsonValue = JsonValue;

    // ── Inspeksi Tipe ──
    [[nodiscard]] Type type() const noexcept;
    [[nodiscard]] bool is_null() const noexcept;
    [[nodiscard]] bool is_bool() const noexcept;
    [[nodiscard]] bool is_integer() const noexcept;
    [[nodiscard]] bool is_double() const noexcept;
    [[nodiscard]] bool is_string() const noexcept;
    [[nodiscard]] bool is_array() const noexcept;
    [[nodiscard]] bool is_object() const noexcept;

    // ── Strict Extraction (mengembalikan Error jika gagal) ──
    [[nodiscard]] Result<bool> as_bool() const noexcept;
    [[nodiscard]] Result<long long> as_integer() const noexcept;
    [[nodiscard]] Result<long double> as_double() const noexcept;
    [[nodiscard]] Result<std::string_view> as_string() const noexcept;

    // ── Fluent Extraction (mengembalikan nilai default jika gagal) ──
    [[nodiscard]] bool get_bool(bool default_val = false) const noexcept;
    [[nodiscard]] long long get_integer(long long default_val = 0) const noexcept;
    [[nodiscard]] double get_double(double default_val = 0.0) const noexcept;
    [[nodiscard]] std::string_view get_string(std::string_view default_val = "") const noexcept;

    // ── Container & Chaining ──
    [[nodiscard]] size_t size() const noexcept;
    [[nodiscard]] bool contains(std::string_view key) const noexcept;

    // Strict traversal
    [[nodiscard]] Result<Value> at(size_t index) const noexcept;
    [[nodiscard]] Result<Value> at(std::string_view key) const noexcept;

    // Fluent traversal (Optional Chaining - mengembalikan Value Null jika tidak ditemukan)
    [[nodiscard]] Value operator[](size_t index) const noexcept;
    [[nodiscard]] Value operator[](std::string_view key) const noexcept;

  private:
    friend class Json;

    const Storage* storage_{nullptr};
    JsonValue value_;

    explicit Value(const Storage* storage, JsonValue value) noexcept;

    [[nodiscard]] static Value fromInternal(const Storage* storage, const JsonValue& v) noexcept;
    [[nodiscard]] static Value createNull(const Storage* storage) noexcept;
};

} // namespace zuu::models