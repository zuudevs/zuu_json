/**
 * @file error.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-05
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

namespace zuu::core {

enum class JsonError : unsigned char {
    None,
    TrailingComma,
    SingleQuotedString,
    UnquotedKey,
    MissingComma,
    UnescapedCharacter,
    InvalidValue,
    CommentNotAllowed,
    InvalidBooleanLiteral,
    InvalidNullLiteral,
    EmptyValue,
    LeadingZero,
    InvalidType,
    IsNotArray,
    IsNotObject,
    RootNotArrayType,
    RootNotObjectType,
    InvalidUnicode,
    InvalidSurrogate,
    Unknown
};

} // namespace zuu::core