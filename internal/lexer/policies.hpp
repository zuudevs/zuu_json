/**
 * @file policies.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Policy definitions for Tokenizer configuration (Compile-time DI)
 * @version 1.0.0
 * @date 2026-06-26
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "lexer/engine/swar.hpp"
#include "lexer/engine/avx2.hpp"

namespace zuu::lexer {

/**
 * @brief Dummy allocator for future extensibility demonstration.
 */
struct DefaultAllocator {};

/**
 * @brief Dummy validator for future extensibility demonstration.
 */
struct FastValidator {};

/**
 * @brief SWAR (SIMD Within A Register) Policy.
 * Ini adalah konfigurasi standar yang menggunakan register 64-bit biasa
 * untuk memproses data secara paralel.
 */
struct SwarPolicy {
    using Engine    = engine::Swar;      // Inject Engine
    using Allocator = DefaultAllocator;  // Inject Allocator (future)
    using Validator = FastValidator;     // Inject Validator (future)
};

struct Avx2Policy {
    using Engine    = engine::Avx2;
    using Allocator = DefaultAllocator;
    using Validator = FastValidator;
};

using DefaultPolicy = SwarPolicy;

} // namespace zuu::lexer