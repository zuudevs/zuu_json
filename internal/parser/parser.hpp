/**
 * @file parser.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Policy-Based Parser Orchestrator
 * @version 1.1.0
 * @date 2026-06-27
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "parser/policies.hpp"
#include <span>

namespace zuu::parser {

/**
 * @brief Parser Engine berbasis Policy.
 * Terinspirasi dari Tokenizer, pengguna dapat menginjeksi backend/engine untuk parser.
 */
template <typename Policy>
class Parser {
  public:
    using Engine = typename Policy::Engine;
    using Expected = typename Engine::Expected;
    using Raw = typename Engine::Raw;
    using Hint = typename Engine::Hint;

    [[nodiscard]] static Expected Parse(Raw tokens, Hint hint) noexcept {
        Engine engine(tokens, hint);
        engine.execute();
        return std::move(engine).result();
    }
};

} // namespace zuu::parser