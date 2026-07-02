/**
 * @file parser.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Policy-Based Fused Parser Orchestrator
 * @version 1.5.0
 * @date 2026-06-29
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "parser/policies.hpp"

namespace zuu::parser {

template <typename Policy = DefaultPolicy>
class Parser {
  public:
    template <typename TokenizerEngine>
    using Engine = typename Policy::template Engine<TokenizerEngine>;

    template <typename TokenizerEngine>
    [[nodiscard]] static std::expected<allocators::Storage, core::JsonError>
        Parse(TokenizerEngine& tokenizer) noexcept {
        Engine<TokenizerEngine> engine(tokenizer);
        engine.execute();
        return std::move(engine).result();
    }
};

} // namespace zuu::parser