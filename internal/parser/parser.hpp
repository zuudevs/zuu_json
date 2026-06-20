/**
 * @file parser.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-10
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "models/storage.hpp"
#include "zuu_json/core/error.hpp"
#include <expected>

namespace zuu::parser {

class Parser {
  public:
    using Token = models::Token;
    using Hint = traits::HintTrait<Token>;
    using TokenType = Token::Type;
    using Error = core::JsonError;
    using Storage = models::Storage;
    using JsonValue = models::JsonValue;
    using JsonMember = models::JsonMember;
    using Raw = std::span<const Token>;
    using Expected = std::expected<Storage, Error>;

    explicit Parser(Raw tokens, Hint hint) noexcept;

    [[nodiscard]] Expected result() && noexcept;
    [[nodiscard]] bool has_error() const noexcept;

    [[nodiscard]] static Expected Parse(Raw tokens, Hint hint) noexcept;

  private:
    Storage res_;
    const Token* current_;
    const Token* end_;
	uint32_t current_depth_{0};
    Error status_{core::JsonError::None};

    static constexpr uint32_t kMaxDepth = 512;

    [[nodiscard]] unsigned decodeUnicodeHex(const char* ptr) noexcept;
    [[nodiscard]] std::string_view unescapeString(std::string_view src) noexcept;

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