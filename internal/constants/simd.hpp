/**
 * @file simd.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-29
 * 
 * @copyright Copyright (c) 2026
 */

#pragma once

#ifdef __AVX2__
#include <immintrin.h>
#endif // __AVX2__

namespace zuu::constants {

alignas(32) static inline const __m256i simd32_msb         = _mm256_set1_epi8(0x80);
alignas(32) static inline const __m256i simd32_31          = _mm256_set1_epi8(0x1F);

alignas(32) static inline const __m256i simd32_dqt         = _mm256_set1_epi8('\"');
alignas(32) static inline const __m256i simd32_esc         = _mm256_set1_epi8('\\');
alignas(32) static inline const __m256i simd32_usc         = _mm256_set1_epi8('_');

alignas(32) static inline const __m256i simd32_ht          = _mm256_set1_epi8('\t');
alignas(32) static inline const __m256i simd32_nl          = _mm256_set1_epi8('\n');
alignas(32) static inline const __m256i simd32_cr          = _mm256_set1_epi8('\r');
alignas(32) static inline const __m256i simd32_sp          = _mm256_set1_epi8(' ');

alignas(32) static inline const __m256i simd32_lcb         = _mm256_set1_epi8('{');
alignas(32) static inline const __m256i simd32_lsb         = _mm256_set1_epi8('[');
alignas(32) static inline const __m256i simd32_rcb         = _mm256_set1_epi8('}');
alignas(32) static inline const __m256i simd32_rsb         = _mm256_set1_epi8(']');
alignas(32) static inline const __m256i simd32_com         = _mm256_set1_epi8(',');

// clang-format off
// PSHUFB Lookup Table untuk Whitespace
alignas(32) static inline const __m256i simd32_lut_whitespace = _mm256_setr_epi8(
	0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x09, 0x0A, 0x00, 0x00, 0x0D, 0x00, 0x00,
	0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x09, 0x0A, 0x00, 0x00, 0x0D, 0x00, 0x00
);
// clang-format on

} // namespace zuu::constants