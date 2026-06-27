/**
 * @file avx2_engine.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief AVX2 specific implementation for Tokenizer Backend
 * @version 1.3.2
 * @date 2026-06-27
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "constants/general.hpp"
#ifdef __AVX2__
#include <immintrin.h>
#endif

#include "tokenizer/tokenizer_base.hpp"
#include "utils/compiler.hpp"
#include <bit>
#include <cstdint>

namespace zuu::tokenizer {

class Avx2Engine : public TokenizerBase<Avx2Engine> {
  public:
    using TokenizerBase<Avx2Engine>::TokenizerBase;
#ifdef __AVX2__
    using block_t = __m256i;
    static inline constexpr uint8_t kBlockSize = sizeof(block_t);

	alignas(32) static inline const __m256i simd32_underscore = _mm256_set1_epi8('_');
	alignas(32) static inline const __m256i simd32_msb        = _mm256_set1_epi8(0x80);
	alignas(32) static inline const __m256i simd32_space      = _mm256_set1_epi8(' ');
	alignas(32) static inline const __m256i simd32_nl         = _mm256_set1_epi8('\n');
	alignas(32) static inline const __m256i simd32_cr         = _mm256_set1_epi8('\r');
	alignas(32) static inline const __m256i simd32_ht         = _mm256_set1_epi8('\t');

	// PSHUFB Lookup Table untuk Whitespace (Pemetaan 4-bit bawah -> nilai Hex aslinya)
	alignas(32) static inline const __m256i simd32_lut_whitespace = _mm256_setr_epi8(
		0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x09, 0x0A, 0x00, 0x00, 0x0D, 0x00, 0x00,
		0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x09, 0x0A, 0x00, 0x00, 0x0D, 0x00, 0x00
	);

	// Konstanta String & Control Character Escape
	alignas(32) static inline const __m256i simd32_doublequote      = _mm256_set1_epi8('\"');
	alignas(32) static inline const __m256i simd32_escape     = _mm256_set1_epi8('\\');
	alignas(32) static inline const __m256i simd32_31         = _mm256_set1_epi8(0x1F);
#endif // __AVX2__

    ZUU_HOT ZUU_ALWAYS_INLINE void skip_whitespace() noexcept {
#ifdef __AVX2__
        while (current_ + kBlockSize <= end_) {
            __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(current_));
            
            __m256i mapped = _mm256_shuffle_epi8(simd32_lut_whitespace, chunk);
            __m256i match  = _mm256_cmpeq_epi8(mapped, chunk);
            uint32_t mask = _mm256_movemask_epi8(match);

            if (mask == ~uint32_t{}) {
                current_ += kBlockSize;
            } else {
                uint32_t non_ws_mask = ~mask;
                current_ += std::countr_zero(non_ws_mask);
                return;
            }
        }
#endif // __AVX2__
        while (current_ < end_ && Lookup{}[*current_] == Lookup::Type::WhiteSpace) {
            ++current_;
        }
    }

    ZUU_HOT ZUU_ALWAYS_INLINE void read_string() noexcept {
        const char* begin = ++current_;
        const char* ptr = begin;
        bool has_escape = false;

#ifdef __AVX2__
        while (ptr + kBlockSize <= end_) {
            __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr));
            
            __m256i eq_quote = _mm256_cmpeq_epi8(chunk, simd32_doublequote);
            __m256i eq_esc   = _mm256_cmpeq_epi8(chunk, simd32_escape);
            __m256i max_ctrl = _mm256_max_epu8(chunk, simd32_31);
            __m256i is_ctrl  = _mm256_cmpeq_epi8(max_ctrl, simd32_31);
            __m256i match = _mm256_or_si256(_mm256_or_si256(eq_quote, eq_esc), is_ctrl);
            uint32_t mask = _mm256_movemask_epi8(match);

            if (mask != constants::zero) {
                uint32_t byte_idx = std::countr_zero(mask);
                auto c = ptr[byte_idx];

                if (c == '\"') [[likely]] {
                    res_.emplace_back(
                        Token::Type::String, 
                        std::string_view(begin, (ptr + byte_idx) - begin), 
                        false 
                    );

                    current_ = ptr + byte_idx + 1;
                    return;
                } else if (static_cast<uint8_t>(c) < 0x20) [[unlikely]] {
                    status_ = Error::UnescapedCharacter;
                    return;
                } else { 
                    ptr += byte_idx;
                    break; 
                }
            }
            ptr += kBlockSize;
        }
#endif // __AVX2__

        this->finish_string_scalar(ptr, begin, has_escape);
    }
};

} // namespace zuu::tokenizer