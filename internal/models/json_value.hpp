/**
 * @file json_value.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-05
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "scalar_storage.hpp"

namespace zuu::models {

class JsonValue {
  public:
    enum class Type : unsigned char {
        Null,
        Boolean,
        Integer,
        Double,
        String,
        Array,
        Object,
    };

    constexpr JsonValue() noexcept : data_(0ull), type_(Type::Null) {}

    [[nodiscard]] static inline constexpr JsonValue Null() {
        return JsonValue{};
    }

    [[nodiscard]] static inline constexpr JsonValue Boolean(long long value) {
        return JsonValue{value};
    }

    [[nodiscard]] static inline constexpr JsonValue Integer(long long value) {
        return JsonValue{value};
    }

    [[nodiscard]] static inline constexpr JsonValue Double(long double value) {
        return JsonValue{value};
    }

    [[nodiscard]] static inline constexpr JsonValue String(unsigned long long index) {
        return JsonValue{index, Type::String};
    }

    [[nodiscard]] static inline constexpr JsonValue Array(unsigned long long index) {
        return JsonValue{index, Type::Array};
    }

    [[nodiscard]] static inline constexpr JsonValue Object(unsigned long long index) {
        return JsonValue{index, Type::Object};
    }

    ScalarStorage data_;
    Type type_;

  private:
    constexpr JsonValue(bool value) noexcept : data_(value), type_(Type::Boolean) {}
    constexpr JsonValue(long long value) noexcept : data_(value), type_(Type::Integer) {}
    constexpr JsonValue(long double value) noexcept : data_(value), type_(Type::Double) {}
    constexpr JsonValue(unsigned long long index, Type type) noexcept : data_(index), type_(type) {}
};

} // namespace zuu::models