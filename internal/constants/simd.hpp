/**
 * @file simd.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-27
 * 
 * @copyright Copyright (c) 2026
 */

#pragma once

#ifdef __AVX2__
#include <immintrin.h>
#endif

namespace zuu::constants {

static inline const __m256i simd32_underscore = _mm256_set1_epi8('_');
static inline const __m256i simd32_msb = _mm256_set1_epi8(0x80);
// static inline const __m256i simd32_underscore = _mm256_set1_epi8('_');
// static inline const __m256i simd32_underscore = _mm256_set1_epi8('_');
// static inline const __m256i simd32_underscore = _mm256_set1_epi8('_');

} // namespace zuu::constants