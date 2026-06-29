/**
 * @file tokenizer.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Policy-Based Tokenizer Orchestrator
 * @version 1.5.0
 * @date 2026-06-29
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "lexer/policies.hpp"
#include <expected>
#include <vector>
#include <span>

namespace zuu::lexer {

/**
 * @brief Lexer Engine berbasis Policy.
 * Hanya digunakan secara eksplisit untuk test/benchmark yang membutuhkan
 * materialisasi `std::vector<Token>`.
 */
template <typename Policy = DefaultPolicy>
class Lexer {
  public:
    using Engine   = typename Policy::Engine;
    using Expected = std::expected<std::vector<models::Token>, core::JsonError>;
    using Raw      = std::span<const char>;

    [[nodiscard]] static Expected Tokenize(Raw json_content) noexcept {
        Engine engine(json_content);
        
        std::vector<models::Token> res;
        res.reserve(json_content.size() >> 1);
        
        while(true) {
            auto tok = engine.next_token();
            if (tok.type_ == models::Token::Type::EndOfFile || engine.is_error()) {
				break;
			}
            res.push_back(tok);
        }

        if (engine.is_error()) {
            return std::unexpected{engine.get_error()};
        }
        
        return res;
    }
};

} // namespace zuu::lexer