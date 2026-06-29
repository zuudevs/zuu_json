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

#include "constants/general.hpp"
#include "tokenizer/tokenizer_base.hpp"
#include "utils/compiler.hpp"
#include "utils/swar.hpp"
#include <bit>
#include <cstdint>

namespace zuu::tokenizer {

class SwarEngine : public TokenizerBase<SwarEngine> {
  public:
    using TokenizerBase<SwarEngine>::TokenizerBase;
	static inline constexpr uint8_t kBlockSize = sizeof(uint64_t);

    [[nodiscard]] Hint pre_scan() noexcept {
        Hint hint{};
        int32_t depth = 0;
        const char* ptr = this->begin_ptr_;
        const char* end = this->end_;

        const uint64_t v_obj = constants::repeat_byte<uint64_t>('{');
        const uint64_t v_arr = constants::repeat_byte<uint64_t>('[');
        const uint64_t v_obj_e = constants::repeat_byte<uint64_t>('}');
        const uint64_t v_arr_e = constants::repeat_byte<uint64_t>(']');
        const uint64_t v_com = constants::repeat_byte<uint64_t>(',');
        const uint64_t v_quo = constants::swar8_doublequote;
        const uint64_t v_esc = constants::swar8_escape;

        while (ptr + 8 <= end) {
            uint64_t chunk{};
            std::memcpy(&chunk, ptr, 8);

            uint64_t m_obj = utils::find_zero_byte_mask(chunk ^ v_obj);
            uint64_t m_arr = utils::find_zero_byte_mask(chunk ^ v_arr);
            uint64_t m_obj_e = utils::find_zero_byte_mask(chunk ^ v_obj_e);
            uint64_t m_arr_e = utils::find_zero_byte_mask(chunk ^ v_arr_e);
            uint64_t m_com = utils::find_zero_byte_mask(chunk ^ v_com);
            uint64_t m_quo = utils::find_zero_byte_mask(chunk ^ v_quo);

            uint64_t mask = m_obj | m_arr | m_obj_e | m_arr_e | m_com | m_quo;

            while (mask != 0) {
                uint32_t bit_pos = std::countr_zero(mask);
                uint32_t byte_pos = bit_pos >> 3;
                char c = ptr[byte_pos];

                if (c == '"') {
                    hint.string_count_++;
                    const char* start = ptr + byte_pos;
                    const char* s_ptr = start + 1;
                    bool has_escape = false;

                    // SWAR Fast String Skip
                    while (s_ptr + 8 <= end) {
                        uint64_t s_chunk{};
                        std::memcpy(&s_chunk, s_ptr, 8);
                        uint64_t sq = utils::find_zero_byte_mask(s_chunk ^ v_quo);
                        uint64_t se = utils::find_zero_byte_mask(s_chunk ^ v_esc);
                        
                        if (se == 0) {
                            if (sq != 0) {
                                s_ptr += (std::countr_zero(sq) >> 3);
                                goto string_ended;
                            }
                            s_ptr += 8;
                        } else {
                            has_escape = true;
                            break;
                        }
                    }

                    // Scalar fallback
                    while (s_ptr < end) {
                        if (*s_ptr == '"') break;
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
                    goto next_chunk; // Restart jendela dari posisi ini
                } else if (c == '{') {
                    hint.object_count_++;
                    if (++depth > this->kMaxDepth) { this->status_ = Error::DepthLimitExceeded; return hint; }
                } else if (c == '[') {
                    hint.array_count_++;
                    if (++depth > this->kMaxDepth) { this->status_ = Error::DepthLimitExceeded; return hint; }
                } else if (c == '}' || c == ']') {
                    depth--;
                } else if (c == ',') {
                    hint.comma_count_++;
                }
                
                mask &= (mask - 1); // Clear bit MSB yang diproses
            }
            ptr += 8;
        next_chunk:;
        }

        // Tail / Fallback untuk sisa byte di ujung file
        while (ptr < end) {
            switch (*ptr) {
                case '{': 
                    hint.object_count_++; 
                    if (++depth > this->kMaxDepth) { this->status_ = Error::DepthLimitExceeded; return hint; }
                    break;
                case '[': 
                    hint.array_count_++; 
                    if (++depth > this->kMaxDepth) { this->status_ = Error::DepthLimitExceeded; return hint; }
                    break;
                case '}': case ']': 
                    depth--; 
                    break;
                case ',': hint.comma_count_++; break;
                case '"': {
                    hint.string_count_++;
                    const char* start = ptr;
                    bool has_escape = false;
                    ++ptr;
                    while (ptr < end) {
                        if (*ptr == '"') break;
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
            uint64_t non_ws = ((block + constants::swar8_underscore) | block) & constants::swar8_msb;
            
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
        const char* ptr = begin;
        const char* end = this->end_;
        bool has_escape = false;

        while (ptr + kBlockSize <= end) {
            uint64_t block{};
            std::memcpy(&block, ptr, kBlockSize);

            uint64_t match_mask = 
                utils::find_zero_byte_mask(block ^ constants::swar8_doublequote) | 
                utils::find_zero_byte_mask(block ^ constants::swar8_escape);

            uint64_t ctrl_mask = 
                (block - constants::swar8_sp) & 
                ~block & constants::swar8_msb;

            uint64_t combined_mask = match_mask | ctrl_mask;
            if (combined_mask != 0) {
                uint32_t byte_idx = static_cast<uint32_t>(std::countr_zero(combined_mask) >> 3);
                char c = ptr[byte_idx];

                if (c == '"') [[likely]] {
                    this->current_ = ptr + byte_idx + 1;
                    return Token(
                        Token::Type::String, 
                        std::string_view(begin, (ptr + byte_idx) - begin), 
                        has_escape
                    );
                } else if (static_cast<uint8_t>(c) < 0x20) [[unlikely]] {
                    this->status_ = Error::UnescapedCharacter; 
                    return Token(Token::Type::Unknown);
                } else { // c == '\\'
                    has_escape = true;
                    ptr += byte_idx + 2;
                    continue; // RESUME SIMD!
                }
            }
            ptr += sizeof(uint64_t);
        }

        return this->finish_string_scalar(ptr, begin, has_escape);
    }
};

} // namespace zuu::tokenizer