/**
 * @file swar.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief SWAR specific implementation for Tokenizer Backend
 * @version 1.5.0
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