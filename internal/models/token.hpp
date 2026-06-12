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

#include "models/hint.hpp"

namespace zuu {

namespace tokenizer {

class Tokenizer;

} // namespace tokenizer

namespace parser {

class Parser;

} // namespace parser

namespace models {

class Token {
  public:
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

    Token(Type type, const char* ptr = "", unsigned size = 0) noexcept
        : ptr_(ptr)
        , size_(size)
        , type_(type) {}

  private:
    const char* ptr_;
    unsigned size_;
    Type type_;

    friend class tokenizer::Tokenizer;
    friend class parser::Parser;
};

template <>
struct Hint<Token> {
	size_t string_count{0};
	size_t array_count{0};
	size_t object_count{0};
	// size_t comma_count{0};
};

} // namespace models

} // namespace zuu