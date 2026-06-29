/**
 * @file avx2_engine.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief AVX2 specific implementation for Tokenizer Backend
 * @version 1.4.0
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

    [[nodiscard]] Hint pre_scan() noexcept {
        Hint hint{};
        int32_t depth = constants::zero;
        const char* ptr = this->begin_ptr_;
        const char* end = this->end_;

#ifdef __AVX2__
        while (ptr + kBlockSize <= end) {
            __m256i chunk   = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr));
            __m256i m_obj   = _mm256_cmpeq_epi8(chunk, constants::simd32_lcb);
            __m256i m_arr   = _mm256_cmpeq_epi8(chunk, constants::simd32_lsb);
            __m256i m_obj_e = _mm256_cmpeq_epi8(chunk, constants::simd32_rcb);
            __m256i m_arr_e = _mm256_cmpeq_epi8(chunk, constants::simd32_rsb);
            __m256i m_com   = _mm256_cmpeq_epi8(chunk, constants::simd32_com);
            __m256i m_quo   = _mm256_cmpeq_epi8(chunk, constants::simd32_dqt);

            __m256i match1  = _mm256_or_si256(
				_mm256_or_si256(m_obj, m_arr), 
				_mm256_or_si256(m_obj_e, m_arr_e)
			);
            __m256i match2  = _mm256_or_si256(m_com, m_quo);
            __m256i match   = _mm256_or_si256(match1, match2);
            uint32_t mask   = _mm256_movemask_epi8(match);

            while (mask != constants::zero) {
                uint32_t bit_pos = std::countr_zero(mask);
                char c = ptr[bit_pos];

                if (c == '\"') {
                    hint.string_count_++;
                    const char* start = ptr + bit_pos;
                    const char* s_ptr = start + 1;
                    bool has_escape = false;

                    while (s_ptr + kBlockSize <= end) {
                        __m256i s_chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(s_ptr));
                        uint32_t m_q    = _mm256_movemask_epi8(_mm256_cmpeq_epi8(s_chunk, constants::simd32_dqt));
                        uint32_t m_e    = _mm256_movemask_epi8(_mm256_cmpeq_epi8(s_chunk, constants::simd32_esc));

                        if (m_e == constants::zero) {
                            if (m_q != constants::zero) {
                                s_ptr += std::countr_zero(m_q);
                                goto string_ended;
                            }
                            s_ptr += kBlockSize;
                        } else {
                            has_escape = true;
                            break;
                        }
                    }

                    while (s_ptr < end) {
                        if (*s_ptr == '\"') {
							break;
						}
                        if (*s_ptr == '\\') {
                            has_escape = true;
                            s_ptr += 2;
                            continue;
                        }
                        s_ptr++;
                    }
                string_ended:
                    if (has_escape) {
                        hint.string_escape_bytes_ += (s_ptr - start);
                    }
                    ptr = s_ptr + 1;
                    goto next_chunk;
                } else if (c == '{') {
                    hint.object_count_++;
                    if (++depth > this->kMaxDepth) { 
						this->status_ = Error::DepthLimitExceeded; 
						return hint; 
					}
                } else if (c == '[') {
                    hint.array_count_++;
                    if (++depth > this->kMaxDepth) { 
						this->status_ = Error::DepthLimitExceeded; 
						return hint; 
					}
                } else if (c == '}' || c == ']') {
                    depth--;
                } else if (c == ',') {
                    hint.comma_count_++;
                }
                
                mask &= (mask - 1);
            }
            ptr += kBlockSize;
        next_chunk:;
        }
#endif // __AVX2__
        while (ptr < end) {
            switch (*ptr) {
                case '{': {
                    hint.object_count_++; 
                    if (++depth > this->kMaxDepth) { 
						this->status_ = Error::DepthLimitExceeded; 
						return hint; 
					}
                    break;
				}
                case '[': {
                    hint.array_count_++; 
                    if (++depth > this->kMaxDepth) { 
						this->status_ = Error::DepthLimitExceeded; 
						return hint; 
					}
                    break;
				}
                case '}': 
				case ']': {
                    depth--;
                    break;
				}
                case ',': {
					hint.comma_count_++; 
					break;
				}
                case '\"': {
                    hint.string_count_++;
                    const char* start = ptr;
                    bool has_escape = false;
                    ++ptr;
                    while (ptr < end) {
                        if (*ptr == '\"') {
							break;
						}
                        if (*ptr == '\\') {
                            has_escape = true;
                            ptr += 2;
                            continue;
                        }
                        ptr++;
                    }
                    if (has_escape) {
                        hint.string_escape_bytes_ += (ptr - start);
                    }
                    break;
                }
                default: break;
            }
            ptr++;
        }
        this->reset();
        return hint;
    }

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
            __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr));
            
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