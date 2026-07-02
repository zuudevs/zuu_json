/**
 * @file token_type.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-07-01
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

namespace zuu::enums {

enum class TokenType : unsigned char {
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

} // namespace zuu::enums