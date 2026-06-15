/**
 * @file token.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-06
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "hint.hpp"
#include <string_view>
namespace zuu::models {

struct Token {
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

    Token(Type type, std::string_view value = "", bool has_escape = false) noexcept
        : begin_(value.data())
        , size_(value.size())
        , type_(type)
		, has_escape_(has_escape) {}
	
	[[nodiscard]] inline constexpr std::string_view 
	value() const noexcept {
		return {begin_, size_};
	}

    const char* begin_;
    unsigned size_;
    Type type_;
	bool has_escape_{false};
};

template <>
struct Hint<Token> {
	size_t string_count_{};
	size_t array_count_{};
	size_t object_count_{};
	size_t comma_count_{};
	size_t string_escape_bytes_{};

	constexpr Hint() noexcept = default;
	constexpr Hint(const Hint&) noexcept = default;
	constexpr Hint(Hint&&) noexcept = default;
	constexpr ~Hint() noexcept = default;
	constexpr Hint& operator=(const Hint&) noexcept = default;
	constexpr Hint& operator=(Hint&&) noexcept = default;
};

} // namespace zuu::models