/**
 * @file error_translator.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-05
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "zuu_json/core/error.hpp"

namespace zuu::utils {

[[nodiscard]] inline constexpr const char* TranslateError(core::JsonError errc) noexcept {
    switch (errc) {
        case core::JsonError::None:
            return "No error";
        case core::JsonError::TrailingComma:
            return "Trailing comma";
        case core::JsonError::SingleQuotedString:
            return "Single quoted string";
        case core::JsonError::UnquotedKey:
            return "Unquoted key";
        case core::JsonError::MissingComma:
            return "Missing comma";
        case core::JsonError::UnescapedCharacter:
            return "Unescaped character";
        case core::JsonError::InvalidValue:
            return "Invalid value";
        case core::JsonError::CommentNotAllowed:
            return "Comment not allowed";
        case core::JsonError::InvalidBooleanLiteral:
            return "Invalid boolean literal";
        case core::JsonError::InvalidNullLiteral:
            return "Invalid null literal";
        case core::JsonError::EmptyValue:
            return "Empty value";
        case core::JsonError::LeadingZero:
            return "Leading zero";
        case core::JsonError::InvalidType:
            return "Invalid type";
        case core::JsonError::IsNotArray:
            return "Not an array";
        case core::JsonError::IsNotObject:
            return "Not an object";
        case core::JsonError::RootNotArrayType:
            return "Root is not array";
        case core::JsonError::RootNotObjectType:
            return "Root is not object";
        case core::JsonError::InvalidUnicode:
            return "Invalid unicode sequence";
        case core::JsonError::InvalidSurrogate:
            return "Invalid unicode surrogate pair";
        default:
            return "Unknown error";
    }
}

} // namespace zuu::utils