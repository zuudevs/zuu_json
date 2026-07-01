/**
 * @file token.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-06
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include <string_view>

#include "enums/token_type.hpp"

namespace zuu::models {

struct Token {
    Token(enums::TokenType type, std::string_view value = "", bool has_escape = false) noexcept
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
    enums::TokenType type_;
    bool has_escape_{false};
};

} // namespace zuu::models