/**
 * @file json_value.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-05
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include <bit>
#include <cstdint>
#include <string_view>
#include "utils/strings.hpp"

namespace zuu::models {

struct JsonValue {
    enum class Type : uint64_t {
        Null,
        Boolean,
        Integer,
        Double,
        String,
        Array,
        Object,
    };

    static constexpr uint64_t NAN_MASK = 0x7FF8000000000000ULL;
    static constexpr uint64_t PAYLOAD_MASK = 0x0000FFFFFFFFFFFFULL;
    static constexpr uint64_t QUIET_NAN_MASK = 0x7;
    static constexpr uint64_t SIGN_EXTENSION_MASK = 0x10;
    static constexpr uint64_t TAG_SHIFT = 0x30;

    static constexpr uint64_t TAG_NULL = static_cast<uint64_t>(Type::Null);
    static constexpr uint64_t TAG_BOOLEAN = static_cast<uint64_t>(Type::Boolean);
    static constexpr uint64_t TAG_INTEGER = static_cast<uint64_t>(Type::Integer);
    static constexpr uint64_t TAG_STRING = static_cast<uint64_t>(Type::String);
    static constexpr uint64_t TAG_ARRAY = static_cast<uint64_t>(Type::Array);
    static constexpr uint64_t TAG_OBJECT = static_cast<uint64_t>(Type::Object);

    uint64_t data_;

    // Encoding + Factory
    [[nodiscard]] static inline constexpr JsonValue Null() noexcept {
        return JsonValue(NAN_MASK | (TAG_NULL << TAG_SHIFT));
    }
    [[nodiscard]] static inline constexpr JsonValue Boolean(bool value) noexcept {
        return JsonValue(NAN_MASK | (TAG_BOOLEAN << TAG_SHIFT) | (value ? 1 : 0));
    }
    [[nodiscard]] static inline constexpr JsonValue Integer(long long value) noexcept {
        return JsonValue(NAN_MASK | (TAG_INTEGER << TAG_SHIFT) |
                         (static_cast<uint64_t>(value) & PAYLOAD_MASK));
    }
    [[nodiscard]] static inline constexpr JsonValue Double(long double value) noexcept {
        return JsonValue(std::bit_cast<uint64_t>(static_cast<double>(value)));
    }
    [[nodiscard]] static inline constexpr JsonValue String(unsigned long long index) noexcept {
        return JsonValue(NAN_MASK | (TAG_STRING << TAG_SHIFT) | (index & PAYLOAD_MASK));
    }
    [[nodiscard]] static inline constexpr JsonValue Array(unsigned long long index) noexcept {
        return JsonValue(NAN_MASK | (TAG_ARRAY << TAG_SHIFT) | (index & PAYLOAD_MASK));
    }
    [[nodiscard]] static inline constexpr JsonValue Object(unsigned long long index) noexcept {
        return JsonValue(NAN_MASK | (TAG_OBJECT << TAG_SHIFT) | (index & PAYLOAD_MASK));
    }

    [[nodiscard]] inline constexpr auto is_double() const noexcept {
        return (data_ & NAN_MASK) != NAN_MASK;
    }

    [[nodiscard]] inline constexpr Type get_type() const noexcept {
        if (is_double())
            return Type::Double;
        return static_cast<Type>((data_ >> TAG_SHIFT) & QUIET_NAN_MASK);
    }

    [[nodiscard]] inline constexpr auto get_payload() const noexcept {
        return data_ & PAYLOAD_MASK;
    }

    // Decoding
    [[nodiscard]] inline constexpr auto as_bool() const noexcept {
        return get_payload() != 0;
    }
    [[nodiscard]] inline constexpr auto as_integer() const noexcept {
        uint64_t payload = get_payload();
        return static_cast<long long>(payload << SIGN_EXTENSION_MASK) >> SIGN_EXTENSION_MASK;
    }
    [[nodiscard]] inline constexpr auto as_double() const noexcept {
        return std::bit_cast<double>(data_);
    }
    [[nodiscard]] inline constexpr auto as_index() const noexcept {
        return get_payload();
    }
};

} // namespace zuu::models