/**
 * @file swar_engine.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief SWAR specific implementation for Tokenizer Backend
 * @version 1.2.0
 * @date 2026-06-29
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "constants/swar.hpp"
#include "lexer/lexer_base.hpp"
#include "utils/compiler.hpp"
#include "utils/swar.hpp"
#include <bit>
#include <cstdint>

namespace zuu::lexer::engine {

class Swar : public LexerBase<Swar> {
  public:
    using LexerBase<Swar>::LexerBase;
	using block_t = uint64_t;
	static inline constexpr uint8_t kBlockSize = sizeof(block_t);

    [[nodiscard]] Hint pre_scan() noexcept {
        Hint hint{};
        int32_t depth = constants::zero;
        const char* ptr = this->begin_ptr_;
        const char* end = this->end_;

        while (ptr + kBlockSize <= end) {
            uint64_t chunk{};
            std::memcpy(&chunk, ptr, kBlockSize);

            uint64_t m_obj   = utils::find_zero_byte_mask(chunk ^ constants::swar8_lcb);
            uint64_t m_arr   = utils::find_zero_byte_mask(chunk ^ constants::swar8_lsb);
            uint64_t m_obj_e = utils::find_zero_byte_mask(chunk ^ constants::swar8_rcb);
            uint64_t m_arr_e = utils::find_zero_byte_mask(chunk ^ constants::swar8_rsb);
            uint64_t m_com   = utils::find_zero_byte_mask(chunk ^ constants::swar8_com);
            uint64_t m_quo   = utils::find_zero_byte_mask(chunk ^ constants::swar8_dqt);
            uint64_t mask    = m_obj | m_arr | m_obj_e | m_arr_e | m_com | m_quo;

            while (mask != constants::zero) {
                uint32_t bit_pos  = std::countr_zero(mask);
                uint32_t byte_pos = bit_pos >> 3;
                char c            = ptr[byte_pos];

                if (c == '\"') {
                    hint.string_count_++;

                    const char* start = ptr + byte_pos;
                    const char* s_ptr = start + 1;
                    bool has_escape   = false;

                    while (s_ptr + kBlockSize <= end) {
                        uint64_t s_chunk{};
                        std::memcpy(&s_chunk, s_ptr, kBlockSize);

                        uint64_t sq = utils::find_zero_byte_mask(s_chunk ^ constants::swar8_dqt);
                        uint64_t se = utils::find_zero_byte_mask(s_chunk ^ constants::swar8_esc);
                        
                        if (se == constants::zero) {
                            if (sq != constants::zero) {
                                s_ptr += (std::countr_zero(sq) >> 3);
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
                    bool has_escape   = false;
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
        while (current_ + kBlockSize <= end_) {
            uint64_t block{};
            std::memcpy(&block, current_, kBlockSize);

            uint64_t non_ws = ((block + constants::swar8_usc) | block) & constants::swar8_msb;
            
            if (non_ws == constants::zero) {
                current_ += kBlockSize;
            } else {
                auto byte_idx = static_cast<uint32_t>(std::countr_zero(non_ws) >> 3);
                current_ += byte_idx;
                break;
            }
        }

        while (current_ < end_ && Lookup{}[*current_] == Lookup::Type::WhiteSpace) {
            ++current_;
        }
    }

    ZUU_HOT ZUU_ALWAYS_INLINE Token read_string() noexcept {
        const char* begin = ++this->current_;
        const char* ptr   = begin;
        const char* end   = this->end_;
        bool has_escape   = false;

        while (ptr + kBlockSize <= end) {
            uint64_t block{};
            std::memcpy(&block, ptr, kBlockSize);

            uint64_t match_mask = 
                utils::find_zero_byte_mask(block ^ constants::swar8_dqt) | 
                utils::find_zero_byte_mask(block ^ constants::swar8_esc);

            uint64_t ctrl_mask  = 
                (block - constants::swar8_sp) & 
                ~block & constants::swar8_msb;

            uint64_t combined_mask = match_mask | ctrl_mask;
            if (combined_mask != constants::zero) {
                auto byte_idx = static_cast<uint32_t>(std::countr_zero(combined_mask) >> 3);
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
            ptr += sizeof(uint64_t);
        }

        return this->finish_string_scalar(ptr, begin, has_escape);
    }
};

} // namespace zuu::lexer::engine