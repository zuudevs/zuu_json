/**
 * @file policies.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-26
 * 
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "zuu_json/core/engine.hpp"

namespace zuu::models {

struct Policy {
	using TokenizerEngine = zuu::core::TokenizerEngine;
	TokenizerEngine tokenizer_engine = TokenizerEngine::Swar;

	constexpr Policy() noexcept = default;
	constexpr Policy(TokenizerEngine tokenizer_engine) noexcept
	 : tokenizer_engine(tokenizer_engine) {}
};

static inline constexpr auto default_policy = Policy{};

} // namespace zuu::models