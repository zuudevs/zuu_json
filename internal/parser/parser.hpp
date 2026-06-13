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

#include <expected>
#include "models/storage.hpp"
#include "models/token.hpp"
#include <span>
#include <string>
#include "zuu_json/core/error.hpp"

namespace zuu::parser {

class Parser {
  public:
    using Token = models::Token;
	using Hint = models::Hint<Token>;
    using TokenType = Token::Type;
    using Error = core::JsonError;
    using Storage = models::Storage;
    using JsonValue = models::JsonValue;
    using JsonMember = models::JsonMember;
    using Raw = std::span<const Token>;
	using Resource = std::pair<Raw, Hint>;
    using Expected = std::expected<Storage, Error>;

    explicit Parser(Resource resource) noexcept;

    [[nodiscard]] Expected result() && noexcept;
    [[nodiscard]] bool has_error() const noexcept;

    [[nodiscard]] static Expected Parse(Resource resource) noexcept;

  private:
    Raw raw_;
    Storage res_;
    size_t idx_{};
    Error status_{core::JsonError::None};

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