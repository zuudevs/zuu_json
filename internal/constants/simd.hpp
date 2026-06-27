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

#ifdef __AVX2__
alignas(32) static inline const __m256i simd32_underscore = _mm256_set1_epi8('_');
alignas(32) static inline const __m256i simd32_msb        = _mm256_set1_epi8(0x80);

// Konstanta Whitespace
alignas(32) static inline const __m256i simd32_space      = _mm256_set1_epi8(' ');
alignas(32) static inline const __m256i simd32_nl         = _mm256_set1_epi8('\n');
alignas(32) static inline const __m256i simd32_cr         = _mm256_set1_epi8('\r');
alignas(32) static inline const __m256i simd32_tab        = _mm256_set1_epi8('\t');

// Konstanta String & Control Character Escape
alignas(32) static inline const __m256i simd32_quote      = _mm256_set1_epi8('"');
alignas(32) static inline const __m256i simd32_escape     = _mm256_set1_epi8('\\');
alignas(32) static inline const __m256i simd32_ctrl_space = _mm256_set1_epi8(0x20);
alignas(32) static inline const __m256i simd32_neg1       = _mm256_set1_epi8(-1);
#endif

} // namespace zuu::constants