/**
 * @file parser.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-10
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "models/storage.hpp"
#include "models/token.hpp"
#include "zuu_json/core/error.hpp"
#include <expected>
#include <span>
#include <string>

namespace zuu::parser {

class Parser {
  public:
    using Token = models::Token;
    using TokenType = Token::Type;
    using Error = core::JsonError;
    using Storage = models::Storage;
    using JsonValue = models::JsonValue;
    using JsonMember = models::JsonMember;
    using Raw = std::span<const Token>;
    using Expected = std::expected<Storage, Error>;

    explicit Parser(Raw tokens) noexcept;

    [[nodiscard]] Expected result() && noexcept;
    [[nodiscard]] bool has_error() const noexcept;

    [[nodiscard]] static Expected Parse(Raw tokens) noexcept;

  private:
    Storage res_;
	const Token* current_;
    const Token* end_;
    Error status_{core::JsonError::None};

    [[nodiscard]] size_t getStringCount() const noexcept;
    [[nodiscard]] size_t getArrayCount() const noexcept;
    [[nodiscard]] size_t getObjectCount() const noexcept;

    [[nodiscard]] Token::Type peek() const noexcept;
    void advance() noexcept;

    [[nodiscard]] std::string parseStringToken(const Token& token) noexcept;

    [[nodiscard]] JsonValue buildNull() noexcept;
    [[nodiscard]] JsonValue buildBoolean() noexcept;
    [[nodiscard]] JsonValue buildInteger() noexcept;
    [[nodiscard]] JsonValue buildDouble() noexcept;
    [[nodiscard]] JsonValue buildString() noexcept;
    [[nodiscard]] JsonValue buildArray() noexcept;
    [[nodiscard]] JsonValue buildObject() noexcept;
    [[nodiscard]] JsonValue buildValue() noexcept;

    void parse() noexcept;
};

} // namespace zuu::parser