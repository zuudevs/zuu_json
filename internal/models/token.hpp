/**
 * @file token.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Tape-based Token definition packed into 64-bit word.
 * @version 1.1.0
 * @date 2026-06-26
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "traits/lookup_trait.hpp"
#include "traits/hint_trait.hpp"
#include <string_view>
#include <cstdint>

namespace zuu {

namespace models {

/**
 * @brief 64-bit Packed Token (Tape Element)
 * Layout: [Type: 8b] [Escape: 1b] [Length: 23b] [Offset: 32b]
 */
struct alignas(8) Token {
    enum class Type : unsigned char {
        LeftCurlyBracket,
        RightCurlyBracket,
        LeftSquareBracket,
        RightSquareBracket,
        Colon,
        Comma,
        Null,
        Boolean,
        Integer,
        Double,
        String,
        EndOfFile,
        Unknown,
    };

    uint64_t data_{0};

    constexpr Token() noexcept = default;

    constexpr Token(Type type, uint32_t offset = 0, uint32_t length = 0, bool has_escape = false) noexcept
	 : data_(
		(static_cast<uint64_t>(type) << 56) |
		(static_cast<uint64_t>(has_escape) << 55) |
		((static_cast<uint64_t>(length) & 0x7FFFFF) << 32) |
		(static_cast<uint64_t>(offset) & 0xFFFFFFFF)
	 ) {}

    [[nodiscard]] inline constexpr Type type() const noexcept { 
        return static_cast<Type>(data_ >> 56); 
    }
    
    [[nodiscard]] inline constexpr bool has_escape() const noexcept { 
        return (data_ >> 55) & 1; 
    }
    
    [[nodiscard]] inline constexpr uint32_t length() const noexcept { 
        return (data_ >> 32) & 0x7FFFFF; 
    }
    
    [[nodiscard]] inline constexpr uint32_t offset() const noexcept { 
        return data_ & 0xFFFFFFFF; 
    }

    // Tape resolution requires the base pointer of the JSON string
    [[nodiscard]] inline constexpr std::string_view value(const char* base) const noexcept {
        return {base + offset(), length()};
    }

    [[nodiscard]] inline constexpr const char* begin(const char* base) const noexcept {
        return base + offset();
    }
};

static_assert(sizeof(Token) == 8, "Token must be exactly 8 bytes for tape optimization");

} // namespace models

namespace traits {

template <>
struct HintTrait<models::Token> {
    unsigned long long string_count_{};
    unsigned long long array_count_{};
    unsigned long long object_count_{};
    unsigned long long comma_count_{};
    unsigned long long string_escape_bytes_{};

    constexpr HintTrait() noexcept = default;
    constexpr HintTrait(const HintTrait&) noexcept = default;
    constexpr HintTrait(HintTrait&&) noexcept = default;
    constexpr ~HintTrait() noexcept = default;
    constexpr HintTrait& operator=(const HintTrait&) noexcept = default;
    constexpr HintTrait& operator=(HintTrait&&) noexcept = default;
};

template <>
struct LookupTrait<models::Token> {
    // clang-format off
	/**
	* @brief lookup table token action
	*
	* > 0 = whitespace
	* > 1 = {
	* > 2 = }
	* > 3 = [
	* > 4 = ]
	* > 5 = :
	* > 6 = ,
	* > 7 = "
	* > 8 = digit/-
	* > 9 = alpha
	* > 10 = '
	* > 255 = error
	*/
	// NOLINTNEXTLINE(modernize-avoid-c-arrays)
	alignas(64) static inline constexpr unsigned char value[256] = {
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0x00, 0xff, 0x07, 0xff, 0xff, 0xff, 0xff, 0x0a, 0xff, 0xff, 0xff, 0xff, 0x06, 0x08, 0xff, 0xff,
		0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x05, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
		0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x03, 0xff, 0x04, 0xff, 0xff,
		0xff, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
		0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x01, 0xff, 0x02, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
	};
    // clang-format on

    enum class Type : unsigned char {
        WhiteSpace,
        LeftCurlyBracket,
        RightCurlyBracket,
        LeftSquareBracket,
        RightSquareBracket,
        Colon,
        Comma,
        DoubleQuote,
        Numeric,
        Alphabet,
        SigleQuote,
        Error
    };

    [[nodiscard]] static inline constexpr Type operator[](unsigned char idx) noexcept {
        return static_cast<Type>(value[idx]);
    }
};

} // namespace traits
} // namespace zuu