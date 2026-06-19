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
#include "constants/general.hpp"
#include <string_view>

namespace zuu::models {

struct JsonValue {
    enum class Type : unsigned char {
        Null,
        Boolean,
        Integer,
        Double,
        String,
        Array,
        Object,
		ShortString 
    };

    using Data = unsigned long long;

    static constexpr Data NAN_MASK = 0x7FF8000000000000ULL;
    static constexpr Data PAYLOAD_MASK = 0x0000FFFFFFFFFFFFULL;
    static constexpr unsigned char QUIET_NAN_MASK = 0x7;
    static constexpr unsigned char SIGN_EXTENSION_MASK = 0x10;
    static constexpr unsigned char TAG_SHIFT = 0x30;

    static constexpr Data TAG_NULL = static_cast<Data>(Type::Null);
    static constexpr Data TAG_BOOLEAN = static_cast<Data>(Type::Boolean);
    static constexpr Data TAG_INTEGER = static_cast<Data>(Type::Integer);
    static constexpr Data TAG_STRING = static_cast<Data>(Type::String);
    static constexpr Data TAG_ARRAY = static_cast<Data>(Type::Array);
    static constexpr Data TAG_OBJECT = static_cast<Data>(Type::Object);
	static constexpr Data TAG_SHORT_STRING = static_cast<Data>(Type::ShortString);

    Data data_;

    // Encoding + Factory
    [[nodiscard]] static inline constexpr JsonValue Null() noexcept {
        return JsonValue(NAN_MASK | (TAG_NULL << TAG_SHIFT));
    }
    [[nodiscard]] static inline constexpr JsonValue Boolean(bool value) noexcept {
        return JsonValue(NAN_MASK | (TAG_BOOLEAN << TAG_SHIFT) | (value ? 1 : 0));
    }
    [[nodiscard]] static inline constexpr JsonValue Integer(long long value) noexcept {
        return JsonValue(NAN_MASK | (TAG_INTEGER << TAG_SHIFT) |
                         (static_cast<Data>(value) & PAYLOAD_MASK));
    }
    [[nodiscard]] static inline constexpr JsonValue Double(long double value) noexcept {
        return JsonValue(std::bit_cast<Data>(static_cast<double>(value)));
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

	[[nodiscard]] static inline constexpr JsonValue ShortString(std::string_view s) noexcept {
        Data payload = (static_cast<Data>(s.size()) << 40); // Size di Byte ke-5
        switch (s.size()) {
            case 5: payload |= (static_cast<Data>(static_cast<unsigned char>(s[4])) << 32); [[fallthrough]];
            case 4: payload |= (static_cast<Data>(static_cast<unsigned char>(s[3])) << 24); [[fallthrough]];
            case 3: payload |= (static_cast<Data>(static_cast<unsigned char>(s[2])) << 16); [[fallthrough]];
            case 2: payload |= (static_cast<Data>(static_cast<unsigned char>(s[1])) << 8);  [[fallthrough]];
            case 1: payload |= (static_cast<Data>(static_cast<unsigned char>(s[0])));
        }
        return JsonValue(NAN_MASK | (TAG_SHORT_STRING << TAG_SHIFT) | payload);
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
        Data payload = get_payload();
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