/**
 * @file tokenizer.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-06
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "models/token.hpp"
#include "zuu_json/core/error.hpp"
#include <expected>
#include <span>
#include <vector>

namespace zuu::tokenizer {

class Tokenizer {
  public:
    explicit Tokenizer(std::span<const char> json_content) noexcept;
    [[nodiscard]] std::expected<std::pair<std::vector<models::Token>, models::Hint<models::Token>>,
                                core::JsonError>
    result() && noexcept;

    [[nodiscard]] static std::expected<
        std::pair<std::vector<models::Token>, models::Hint<models::Token>>,
        core::JsonError>
    Tokenize(std::span<const char> json_content) noexcept;

  private:
    std::vector<models::Token> res_;
    models::Hint<models::Token> hint_{};
    const char* current_;
    const char* end_;
    core::JsonError status_{core::JsonError::None};

    [[nodiscard]] bool is_error() const noexcept;
    void readString() noexcept;
    void readNumeric() noexcept;
    void readAlphabet() noexcept;
    void tokenize() noexcept;
};

} // namespace zuu::tokenizer