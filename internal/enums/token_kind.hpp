/**
 * @file token_kind.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-07-01
 * 
 * @copyright Copyright (c) 2026
 */

#pragma once

namespace zuu::enums {

enum class TokenKind : unsigned char {
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

} // namespace zuu::enums