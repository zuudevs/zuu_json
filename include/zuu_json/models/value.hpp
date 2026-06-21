/**
 * @file value.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-05
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "models/array_view.hpp"
#include "models/member_view.hpp"
#include "models/object_view.hpp"
#include "models/json_value.hpp"
#include <expected>
#include <string_view>
#include "zuu_json/core/error.hpp"

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
    [[nodiscard]] unsigned long long size() const noexcept;
    [[nodiscard]] bool contains(std::string_view key) const noexcept;

	// ── Iterators & Views ──
    [[nodiscard]] Result<ArrayView> as_array() const noexcept;
    [[nodiscard]] Result<ObjectView> as_object() const noexcept;
    [[nodiscard]] ArrayView get_array() const noexcept;
    [[nodiscard]] ObjectView get_object() const noexcept;

	// ── Serializer / Emitter ──
    [[nodiscard]] std::string dump(int indent = -1) const noexcept;

    // Strict traversal
    [[nodiscard]] Result<Value> at(unsigned long long index) const noexcept;
    [[nodiscard]] Result<Value> at(std::string_view key) const noexcept;

    // Fluent traversal (Optional Chaining - mengembalikan Value Null jika tidak ditemukan)
    [[nodiscard]] Value operator[](unsigned long long index) const noexcept;
    [[nodiscard]] Value operator[](std::string_view key) const noexcept;

  private:
    friend class Json;
    friend class ArrayView;
    friend class ObjectView;
	friend struct MemberView;

    const Storage* storage_{nullptr};
    JsonValue value_;

    explicit Value(const Storage* storage, JsonValue value) noexcept;

    [[nodiscard]] static Value fromInternal(const Storage* storage, const JsonValue& v) noexcept;
    [[nodiscard]] static Value createNull(const Storage* storage) noexcept;
};

template <std::size_t N>
[[nodiscard]] decltype(auto) get(const MemberView& m) noexcept {
    if constexpr (N == 0) { return m.key_; }
    else if constexpr (N == 1) { return m.value(); }
}

} // namespace zuu::models

namespace std {
    template <>
    struct tuple_size<zuu::models::MemberView> : std::integral_constant<std::size_t, 2> {};

    template <>
    struct tuple_element<0, zuu::models::MemberView> { using type = std::string_view; };

    template <>
    struct tuple_element<1, zuu::models::MemberView> { using type = zuu::models::Value; };
}