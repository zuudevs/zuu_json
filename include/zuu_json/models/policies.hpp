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
	zuu::core::TokenizerEngine tokenizer_engine = core::TokenizerEngine::Swar;

	constexpr Policy() noexcept = default;
};

static inline constexpr auto default_policy = Policy{};

} // namespace zuu::models