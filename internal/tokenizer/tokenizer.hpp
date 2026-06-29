/**
 * @file tokenizer.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Policy-Based Tokenizer Orchestrator (Backward Compatibility)
 * @version 1.2.0
 * @date 2026-06-29
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "tokenizer/policies.hpp"
#include <expected>
#include <vector>
#include <span>

namespace zuu::tokenizer {

/**
 * @brief Tokenizer Engine berbasis Policy.
 * Hanya digunakan secara eksplisit untuk test/benchmark yang membutuhkan
 * materialisasi `std::vector<Token>`.
 */
template <typename Policy>
class Tokenizer {
  public:
    using Engine = typename Policy::Engine;
    using Expected = std::expected<std::pair<std::vector<models::Token>, traits::HintTrait<models::Token>>, core::JsonError>;
    using Raw = std::span<const char>;

    [[nodiscard]] static Expected Tokenize(Raw json_content) noexcept {
        Engine engine(json_content);
        
        auto hint = engine.pre_scan();
        if (engine.is_error()) return std::unexpected{engine.get_error()};

        std::vector<models::Token> res;
        res.reserve((json_content.size() >> 1) + constants::word);
        
        while(true) {
            auto tok = engine.next_token();
            if (tok.type_ == models::Token::Type::EndOfFile || engine.is_error()) break;
            res.push_back(tok);
        }

        if (engine.is_error()) {
            return std::unexpected{engine.get_error()};
        }
        
        return std::pair{std::move(res), hint};
    }
};

} // namespace zuu::tokenizer