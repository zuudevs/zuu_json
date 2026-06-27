/**
 * @file swar_engine.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief SWAR specific implementation for Tokenizer Backend
 * @version 1.1.0
 * @date 2026-06-26
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

    ZUU_HOT ZUU_ALWAYS_INLINE void read_string() noexcept {
        const char* begin = ++current_;
        const char* ptr = begin;
        const char* end = end_;
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
                auto byte_idx = static_cast<uint32_t>(std::countr_zero(combined_mask) >> 3);
                auto c = ptr[byte_idx];

                if (static_cast<uint8_t>(c) < 0x20) {
                    status_ = Error::UnescapedCharacter; 
                    return;
                } else if (c == '\"') {
                    res_.emplace_back(
                        Token::Type::String, 
                        std::string_view(begin, (ptr + byte_idx) - begin), 
                        has_escape
                    );

                    if (has_escape) {
                        hint_.string_escape_bytes_ += ((ptr + byte_idx) - begin);
                    }

                    current_ = ptr + byte_idx + 1;
                    return;
                } else {
                    ptr += byte_idx;
                    break;
                }
            }
            ptr += sizeof(uint64_t);
        }

        this->finish_string_scalar(ptr, begin, has_escape);
    }
};

} // namespace zuu::tokenizer