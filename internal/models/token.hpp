/**
 * @file token.hpp
 * @author zuu::modelsdevs (zuu::modelsdevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-06
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "models/hint.hpp"
#include <string_view>

namespace zuu::models {

struct Token {
    static constexpr unsigned char layout_required_size = 16;

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
        : ptr_(value.data())
        , size_(static_cast<uint32_t>(value.size()))
        , type_(type)
        , has_escape_(has_escape) {}

    [[nodiscard]] constexpr std::string_view value() const noexcept {
        return {ptr_, size_};
    }

    const char* ptr_{nullptr}; // 8 bytes — offset 0
    uint32_t size_{0};          // 4 bytes — offset 8
    Type type_;                 // 1 byte  — offset 12
    bool has_escape_{false};    // 1 byte  — offset 13 (Aman, menempati padding sisa)
    // 2 bytes padding
    // sizeof(Token) == 16
};

static_assert(sizeof(Token) == Token::layout_required_size, "Token layout changed unexpectedly");

template <>
struct Hint<Token> {
    size_t string_count{0};
    size_t array_count{0};
    size_t object_count{0};
    size_t comma_count{0};
    size_t string_escape_bytes{0}; // Mencatat total memori maksimal untuk string yang butuh unescaping
};

} // namespace zuu::models