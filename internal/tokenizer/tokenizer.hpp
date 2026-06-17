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
    using Token = models::Token;
    using Lookup = traits::LookupTrait<Token>;
    using Error = core::JsonError;
    using Result = std::vector<Token>;
    using Hint = traits::HintTrait<Token>;
    using Resource = std::pair<Result, Hint>;
    using Expected = std::expected<Resource, Error>;
    using Raw = std::span<const char>;

    explicit Tokenizer(Raw json_content) noexcept;
    [[nodiscard]] Expected result() && noexcept;

    [[nodiscard]] static Expected Tokenize(Raw json_content) noexcept;

  private:
    Result res_;
    Hint hint_{};
    const char* current_;
    const char* end_;
    Error status_{Error::None};

    [[nodiscard]] bool is_error() const noexcept;
    void readString() noexcept;
    void readNumeric() noexcept;
    void readAlphabet() noexcept;
    void tokenize() noexcept;
};

} // namespace zuu::tokenizer