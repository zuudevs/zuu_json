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

#include <string_view>

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

    constexpr Token(Type type, const char* ptr = nullptr, unsigned size = 0) noexcept : ptr_(ptr), size_(size), type_(type) {}
	
	[[nodiscard]] constexpr inline auto 
	value() const noexcept {
		return std::string_view(ptr_, size_);
	}


  private:
    const char* ptr_;
	unsigned size_;
    Type type_;

    friend class tokenizer::Tokenizer;
    friend class parser::Parser;
};

} // namespace models

} // namespace zuu