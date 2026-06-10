/**
 * @file parser.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-06
 *
 * @copyright Copyright (c) 2026
 */

#include "parser/parser.hpp"
#include <charconv>

namespace zuu::parser {

Parser::Parser(Parser::Raw tokens) noexcept
    : raw_(tokens) {
    res_ = Storage(getStringCount(), getArrayCount(), getObjectCount());
    parse();
}

Parser::Expected Parser::result() && noexcept {
    if (has_error()) {
        return std::unexpected{status_};
    }
    return std::move(res_);
}

bool Parser::has_error() const noexcept {
    return status_ != Error::None;
}

Parser::Expected Parser::Parse(Parser::Raw tokens) noexcept {
    return Parser(tokens).result();
}

size_t Parser::getStringCount() const noexcept {
    size_t n{0};
    for (const auto& token : raw_) {
        if (token.type_ == TokenType::String) {
            n++;
        }
    }
    return n;
}

size_t Parser::getArrayCount() const noexcept {
    size_t n{0};
    for (const auto& token : raw_) {
        if (token.type_ == TokenType::LeftSquareBracket) {
            n++;
        }
    }
    return n;
}

size_t Parser::getObjectCount() const noexcept {
    size_t n{0};
    for (const auto& token : raw_) {
        if (token.type_ == TokenType::LeftCurlyBracket) {
            n++;
        }
    }
    return n;
}

Parser::TokenType Parser::peek() const noexcept {
    return raw_[idx_].type_;
}

void Parser::advance() noexcept {
    idx_++;
}

Parser::JsonValue Parser::buildNull() noexcept {
    advance();
    return Parser::JsonValue::Null();
}

Parser::JsonValue Parser::buildBoolean() noexcept {
    const auto value = raw_[idx_].ptr_[0] == 't';
    advance();
    return Parser::JsonValue::Boolean(value);
}

Parser::JsonValue Parser::buildInteger() noexcept {
    long long value{};
    auto [ptr, ec] = std::from_chars(raw_[idx_].ptr_, raw_[idx_].ptr_ + raw_[idx_].size_, value);

    if (ec != std::errc{} || ptr != raw_[idx_].ptr_ + raw_[idx_].size_) {
        status_ = core::JsonError::InvalidValue;
        return Parser::JsonValue::Null();
    }
    advance();
    return Parser::JsonValue::Integer(value);
}

Parser::JsonValue Parser::buildDouble() noexcept {
    long double value{};
    auto [ptr, ec] = std::from_chars(raw_[idx_].ptr_, raw_[idx_].ptr_ + raw_[idx_].size_, value);

    if (ec != std::errc{} || ptr != raw_[idx_].ptr_ + raw_[idx_].size_) {
        status_ = core::JsonError::InvalidValue;
        return Parser::JsonValue::Null();
    }
    advance();
    return Parser::JsonValue::Double(value);
}

Parser::JsonValue Parser::buildString() noexcept {
    const auto index = res_.addString(raw_[idx_].ptr_);
    advance();
    return Parser::JsonValue::String(index);
}

Parser::JsonValue Parser::buildArray() noexcept {
    const auto array_index = res_.addArray();
    advance();

    if (idx_ >= raw_.size()) {
        status_ = core::JsonError::InvalidValue;
        return Parser::JsonValue::Null();
    }

    if (peek() == TokenType::RightSquareBracket) {
        advance();
        return Parser::JsonValue::Array(array_index);
    }

    while (idx_ < raw_.size()) {
        if (peek() == TokenType::RightSquareBracket || peek() == TokenType::Comma) {
            status_ = core::JsonError::EmptyValue;
            return Parser::JsonValue::Null();
        }

        auto value = buildValue();
        if (has_error()) {
            return Parser::JsonValue::Null();
        }

        res_.array(array_index).push_back(value);

        if (idx_ >= raw_.size()) {
            status_ = core::JsonError::InvalidValue;
            return Parser::JsonValue::Null();
        }

        if (peek() == TokenType::Comma) {
            advance();
            if (idx_ < raw_.size() && peek() == TokenType::RightSquareBracket) {
                status_ = core::JsonError::TrailingComma;
                return Parser::JsonValue::Null();
            }
            continue;
        }

        if (peek() == TokenType::RightSquareBracket) {
            advance();
            return Parser::JsonValue::Array(array_index);
        }

        status_ = core::JsonError::MissingComma;
        return Parser::JsonValue::Null();
    }

    status_ = core::JsonError::InvalidValue;
    return Parser::JsonValue::Null();
}

Parser::JsonValue Parser::buildObject() noexcept {
    const auto object_index = res_.addObject();
    advance();

    if (idx_ >= raw_.size()) {
        status_ = core::JsonError::InvalidValue;
        return Parser::JsonValue::Null();
    }

    if (peek() == TokenType::RightCurlyBracket) {
        advance();
        return Parser::JsonValue::Object(object_index);
    }

    while (idx_ < raw_.size()) {
        if (peek() != TokenType::String) {
            status_ = core::JsonError::UnquotedKey;
            return Parser::JsonValue::Null();
        }

        const auto key_index = res_.addString(raw_[idx_].ptr_);
        advance();

        if (idx_ >= raw_.size() || peek() != TokenType::Colon) {
            status_ = core::JsonError::InvalidType;
            return Parser::JsonValue::Null();
        }
        advance();

        if (idx_ >= raw_.size()) {
            status_ = core::JsonError::EmptyValue;
            return Parser::JsonValue::Null();
        }

        if (peek() == TokenType::RightCurlyBracket || peek() == TokenType::Comma) {
            status_ = core::JsonError::EmptyValue;
            return Parser::JsonValue::Null();
        }

        auto value = buildValue();
        if (has_error()) {
            return Parser::JsonValue::Null();
        }

        res_.object(object_index)
            .push_back(Parser::JsonMember{.key_index_ = key_index, .value_ = value});

        if (idx_ >= raw_.size()) {
            status_ = core::JsonError::InvalidValue;
            return Parser::JsonValue::Null();
        }

        if (peek() == TokenType::Comma) {
            advance();
            if (idx_ < raw_.size() && peek() == TokenType::RightCurlyBracket) {
                status_ = core::JsonError::TrailingComma;
                return Parser::JsonValue::Null();
            }
            continue;
        }

        if (peek() == TokenType::RightCurlyBracket) {
            advance();
            return Parser::JsonValue::Object(object_index);
        }

        status_ = core::JsonError::MissingComma;
        return Parser::JsonValue::Null();
    }

    status_ = core::JsonError::InvalidValue;
    return Parser::JsonValue::Null();
}

Parser::JsonValue Parser::buildValue() noexcept {
    if (idx_ >= raw_.size()) {
        status_ = core::JsonError::EmptyValue;
        return Parser::JsonValue::Null();
    }

    switch (peek()) {
        case TokenType::Null:
            return buildNull();
        case TokenType::Boolean:
            return buildBoolean();
        case TokenType::Integer:
            return buildInteger();
        case TokenType::Double:
            return buildDouble();
        case TokenType::String:
            return buildString();
        case TokenType::LeftSquareBracket:
            return buildArray();
        case TokenType::LeftCurlyBracket:
            return buildObject();
        case TokenType::RightSquareBracket:
        case TokenType::RightCurlyBracket:
        case TokenType::Comma:
        case TokenType::Colon:
            status_ = core::JsonError::EmptyValue;
            return Parser::JsonValue::Null();
        default:
            status_ = core::JsonError::InvalidValue;
            return Parser::JsonValue::Null();
    }
}

void Parser::parse() noexcept {
    if (raw_.empty()) {
        status_ = core::JsonError::EmptyValue;
        return;
    }

    auto root = buildValue();
    if (has_error()) {
        return;
    }

    res_.setRoot(root);
    if (idx_ < raw_.size()) {
        status_ = core::JsonError::InvalidValue;
    }
}

} // namespace zuu::parser