/**
 * @file policies.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Policy definitions for Fused Parser configuration
 * @version 1.1.0
 * @date 2026-06-29
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "parser/engine/swar.hpp"
#include "parser/engine/avx2.hpp"

namespace zuu::parser {

struct DefaultAllocator {};
struct FastValidator {};

struct SwarPolicy {
    template <typename LexerEngine>
    using Engine    = engine::Swar<LexerEngine>;
    using Allocator = DefaultAllocator;
    using Validator = FastValidator;
};

struct Avx2Policy {
    template <typename LexerEngine>
    using Engine    = engine::Avx2<LexerEngine>;
    using Allocator = DefaultAllocator;
    using Validator = FastValidator;
};

using DefaultPolicy = SwarPolicy;

} // namespace zuu::parser