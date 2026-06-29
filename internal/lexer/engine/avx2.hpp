/**
 * @file avx2.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief AVX2 specific implementation for Tokenizer Backend
 * @version 1.5.0
 * @date 2026-06-29
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#ifdef __AVX2__
#include <bit>
#include "constants/simd.hpp"
#endif // __AVX2__

#include "lexer/lexer_base.hpp"
#include "utils/compiler.hpp"
#include <cstdint>

namespace zuu::lexer::engine {

class Avx2 : public LexerBase<Avx2> {
  public:
    using LexerBase<Avx2>::LexerBase;
#ifdef __AVX2__
    using block_t = __m256i;
    static inline constexpr uint8_t kBlockSize = sizeof(block_t);
#endif // __AVX2__

    ZUU_HOT ZUU_ALWAYS_INLINE void skip_whitespace() noexcept {
#ifdef __AVX2__
        while (current_ + kBlockSize <= end_) {
            __m256i chunk  = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(current_));
            __m256i mapped = _mm256_shuffle_epi8(constants::simd32_lut_whitespace, chunk);
            __m256i match  = _mm256_cmpeq_epi8(mapped, chunk);
            uint32_t mask  = _mm256_movemask_epi8(match);

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

    ZUU_HOT ZUU_ALWAYS_INLINE Token read_string() noexcept {
        const char* begin = ++this->current_;
        const char* ptr = begin;
        bool has_escape = false;

#ifdef __AVX2__
        while (ptr + kBlockSize <= this->end_) {
            __m256i chunk    = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr));
            __m256i eq_quote = _mm256_cmpeq_epi8(chunk, constants::simd32_dqt);
            __m256i eq_esc   = _mm256_cmpeq_epi8(chunk, constants::simd32_esc);
            __m256i max_ctrl = _mm256_max_epu8(chunk, constants::simd32_31);
            __m256i is_ctrl  = _mm256_cmpeq_epi8(max_ctrl, constants::simd32_31);
            __m256i match = _mm256_or_si256(_mm256_or_si256(eq_quote, eq_esc), is_ctrl);
            uint32_t mask = _mm256_movemask_epi8(match);

            if (mask != constants::zero) {
                uint32_t byte_idx = std::countr_zero(mask);
                char c = ptr[byte_idx];

                if (c == '\"') [[likely]] {
                    this->current_ = ptr + byte_idx + 1;
                    return {
                        Token::Type::String, 
                        std::string_view(begin, (ptr + byte_idx) - begin), 
                        has_escape
					};
                } else if (static_cast<uint8_t>(c) < 0x20) [[unlikely]] {
                    this->status_ = Error::UnescapedCharacter;
                    return Token::Type::Unknown;
                } else {
                    has_escape = true;
                    ptr += byte_idx + 2;
                    continue;
                }
            }
            ptr += kBlockSize;
        }
#endif // __AVX2__

        return this->finish_string_scalar(ptr, begin, has_escape);
    }
};

} // namespace zuu::lexer::engine