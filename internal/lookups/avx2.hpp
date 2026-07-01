/**
 * @file avx2.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-07-01
 * 
 * @copyright Copyright (c) 2026
 */

#pragma once

#ifdef __AVX2__

#include <immintrin.h>

namespace zuu::lookups {

// clang-format off
// PSHUFB Lookup Table untuk Whitespace
alignas(32) static inline const __m256i simd32_whitespace = _mm256_setr_epi8(
	0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x09, 0x0A, 0x00, 0x00, 0x0D, 0x00, 0x00,
	0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x09, 0x0A, 0x00, 0x00, 0x0D, 0x00, 0x00
);
// clang-format on

} // namespace zuu::lookups

#endif // __AVX2__