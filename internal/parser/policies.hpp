/**
 * @file policies.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Policy definitions for Parser configuration (Compile-time DI)
 * @version 1.0.0
 * @date 2026-06-27
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "parser/engine/swar_engine.hpp"
#include "parser/engine/avx2_engine.hpp"

namespace zuu::parser {

struct DefaultAllocator {};
struct FastValidator {};

struct SwarPolicy {
    using Engine    = SwarEngine;
    using Allocator = DefaultAllocator;
    using Validator = FastValidator;
};

struct Avx2Policy {
    using Engine    = Avx2Engine;
    using Allocator = DefaultAllocator;
    using Validator = FastValidator;
};

using DefaultPolicy = SwarPolicy;

} // namespace zuu::parser