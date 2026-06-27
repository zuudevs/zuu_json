/**
 * @file policies.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.2.0
 * @date 2026-06-27
 * 
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "zuu_json/core/engine.hpp"

namespace zuu::models {

struct Policy {
    using TokenizerEngine = zuu::core::TokenizerEngine;
    using ParserEngine = zuu::core::ParserEngine;

    TokenizerEngine tokenizer_engine = TokenizerEngine::Swar;
    ParserEngine parser_engine = ParserEngine::Default;

    constexpr Policy() noexcept = default;
    
    constexpr Policy(
        TokenizerEngine t_engine, 
        ParserEngine p_engine = ParserEngine::Default
    ) noexcept
        : tokenizer_engine(t_engine), parser_engine(p_engine) {}
};

static inline constexpr auto default_policy = Policy{};

} // namespace zuu::models