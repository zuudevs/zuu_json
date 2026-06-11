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

    union Data {
        bool b;
        long long i;
        long double d;
        unsigned long long index;

        constexpr Data(bool value) noexcept
            : b(value) {}
        constexpr Data(long long value) noexcept
            : i(value) {}
        constexpr Data(long double value) noexcept
            : d(value) {}
        constexpr Data(unsigned long long index = 0) noexcept
            : index(index) {}
    };

    [[nodiscard]] static inline JsonValue Null() noexcept {
        return JsonValue{0ull, Type::Null};
    };
    [[nodiscard]] static inline JsonValue Boolean(bool value) noexcept {
        return JsonValue{value};
    };
    [[nodiscard]] static inline JsonValue Integer(long long value) noexcept {
        return JsonValue{value};
    };
    [[nodiscard]] static inline JsonValue Double(long double value) noexcept {
        return JsonValue{value};
    };
    [[nodiscard]] static inline JsonValue String(unsigned long long index) noexcept {
        return JsonValue{index, Type::String};
    };
    [[nodiscard]] static inline JsonValue Array(unsigned long long index) noexcept {
        return JsonValue{index, Type::Array};
    };
    [[nodiscard]] static inline JsonValue Object(unsigned long long index) noexcept {
        return JsonValue{index, Type::Object};
    };

    Data data_{};
    Type type_{};

    constexpr JsonValue() noexcept
        : data_(0ll)
        , type_(Type::Null) {}
    constexpr JsonValue(bool value) noexcept
        : data_(value)
        , type_(Type::Boolean) {}
    constexpr JsonValue(long long value) noexcept
        : data_(value)
        , type_(Type::Integer) {}
    constexpr JsonValue(long double value) noexcept
        : data_(value)
        , type_(Type::Double) {}
    constexpr JsonValue(unsigned long long index, Type type) noexcept
        : data_(index)
        , type_(type) {}
};

} // namespace zuu::models