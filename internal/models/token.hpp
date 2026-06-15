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

    Token(Type type, const char* begin = "", unsigned size = 0) noexcept
        : begin_(begin)
        , size_(size)
        , type_(type) {}

    const char* begin_;
    unsigned size_;
    Type type_;
};

template <>
struct Hint<Token> {
	size_t string_count_{};
	size_t array_count_{};
	size_t object_count_{};

	constexpr Hint() noexcept = default;
	constexpr Hint(const Hint&) noexcept = delete;
	constexpr Hint(Hint&&) noexcept = default;
	constexpr ~Hint() noexcept = default;
	constexpr Hint& operator=(const Hint&) noexcept = delete;
	constexpr Hint& operator=(Hint&&) noexcept = default;
};

} // namespace zuu::models