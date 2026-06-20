/**
 * @file tokenizer.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-06
 *
 * @copyright Copyright (c) 2026
 */

#include <bit>
#include <cstdint>
#include <cstring>
#include "tokenizer/tokenizer.hpp"
#include "constants/general.hpp"
#include "utils/swar.hpp"

namespace zuu::tokenizer {

Tokenizer::Tokenizer(std::span<const char> json_content) noexcept
    : current_(json_content.data())
    , end_(json_content.data() + json_content.size()) {

    res_.reserve((json_content.size() >> 1) + constants::word);
    tokenize();
}

Tokenizer::Expected Tokenizer::result() && noexcept {
    if (is_error()) {
        return std::unexpected{status_};
    }
    return std::pair{std::move(res_), hint_};
}

Tokenizer::Expected Tokenizer::Tokenize(Raw json_content) noexcept {
    return Tokenizer(json_content).result();
}

bool Tokenizer::is_error() const noexcept {
    return status_ != Error::None;
}

void Tokenizer::readString() noexcept {
    const char* begin = ++current_;
    const char* ptr = begin;
    const char* end = end_;
    bool has_escape = false;

    while (ptr + sizeof(uint64_t) <= end) {
		uint64_t block{};
		std::memcpy(&block, ptr, sizeof(uint64_t));

		uint64_t match_mask = 
		utils::find_zero_byte_mask(block ^ constants::swar8_doublequote) | 
		utils::find_zero_byte_mask(block ^ constants::swar8_escape) ;

		uint64_t ctrl_mask = 
		(block - constants::swar8_sp) & 
		~block & constants::swar8_msb ;

        uint64_t combined_mask = match_mask | ctrl_mask;
		if (combined_mask != 0) {
			uint32_t byte_idx = std::countr_zero(combined_mask) >> 3;
			auto c = ptr[byte_idx];

			if (static_cast<uint8_t>(c) < 0x20) {
				status_ = Error::UnescapedCharacter; 
                return;
			} else if (ptr[byte_idx] == '\"') {
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

    while (ptr < end) {
        char c = *ptr;
		if (static_cast<unsigned char>(c) < 0x20) {
            status_ = Error::UnescapedCharacter;
            return;
        }

        if (c == '\"') {
            res_.emplace_back(
                Token::Type::String, 
				std::string_view(begin, ptr - begin), 
				has_escape
			);

            if (has_escape) {
                hint_.string_escape_bytes_ += (ptr - begin);
            }

            current_ = ptr + 1;
            return;
        }
        if (c == '\\') [[unlikely]] {
            has_escape = true;
            ptr += 2;
            if (ptr > end) {
                status_ = core::JsonError::InvalidValue;
                return;
            }
            continue;
        }
        ++ptr;
    }

    status_ = core::JsonError::InvalidValue;
}

void Tokenizer::readNumeric() noexcept {
    const char* begin = current_;
    auto type = Token::Type::Integer;

    auto skip_digits = [this]() noexcept {
        while (current_ + sizeof(uint64_t) <= end_) {
            uint64_t block{};
            std::memcpy(&block, current_, sizeof(uint64_t));
            
            uint64_t val = block - constants::swar8_zero;
            uint64_t non_digits = ((val + constants::swar8_digit_bias) | val) & constants::swar8_msb;
            if (non_digits == constants::zero) {
                current_ += sizeof(uint64_t);
            } else {
                current_ += std::countr_zero(non_digits) >> 3;
                break;
            }
        }
        
        while (current_ < end_ && static_cast<unsigned char>(*current_ - '0') < constants::digit) {
            current_++;
        }
    };

    if (*current_ == '-') {
        current_++;
        if (current_ == end_) { 
			status_ = Error::InvalidValue; 
			return; 
		}
    }

    if (*current_ == '0') {
        current_++;
        if (current_ < end_ && static_cast<unsigned char>(*current_ - '0') < constants::digit) {
            status_ = Error::LeadingZero;
            return;
        }
    } else if (static_cast<unsigned char>(*current_ - '0') < constants::digit) {
        skip_digits();
    } else {
        status_ = Error::InvalidValue;
        return;
    }

    if (current_ < end_ && *current_ == '.') {
        type = Token::Type::Double;
        current_++;
        
        if (current_ == end_ || static_cast<unsigned char>(*current_ - '0') >= constants::digit) {
            status_ = Error::InvalidValue;
            return;
        }
        skip_digits();
    }

    if (current_ < end_ && (*current_ == 'e' || *current_ == 'E')) {
        type = Token::Type::Double;
        current_++;
        
        if (current_ < end_ && (*current_ == '+' || *current_ == '-')) {
            current_++;
        }
        
        if (current_ == end_ || static_cast<unsigned char>(*current_ - '0') >= constants::digit) {
            status_ = Error::InvalidValue;
            return;
        }
        skip_digits();
    }

    if (!is_error()) [[likely]] {
        res_.emplace_back(type, std::string_view(begin, current_ - begin));
    }
}

void Tokenizer::readAlphabet() noexcept {
    const auto rem = end_ - current_;
    
    if (rem >= constants::nibble) [[likely]] {
        uint32_t val{};
        std::memcpy(&val, current_, constants::nibble);

        switch (val) {
            case constants::null_word:
                res_.emplace_back(
					Token::Type::Null, 
					std::string_view(current_, constants::nibble)
				);
                current_ += constants::nibble;
                return;
                
            case constants::true_word:
                res_.emplace_back(
					Token::Type::Boolean, 
					std::string_view(current_, constants::nibble)
				);
                current_ += constants::nibble;
                return;
                
            case constants::fals_word:
				if (rem >= 5 && current_[4] == 'e') {
                    res_.emplace_back(
                        Token::Type::Boolean, 
                        std::string_view(current_, 5)
                    );
                    current_ += 5;
                    return;
                }
			default:
				break;
        }
    }
	status_ = Error::InvalidValue;
}

void Tokenizer::tokenize() noexcept {
    while (current_ < end_) {
        auto actionType = Lookup{}[*current_];
        switch (actionType) {
            case Lookup::Type::WhiteSpace: {
				while (current_ + sizeof(uint64_t) <= end_) {
                    uint64_t block{};
                    std::memcpy(&block, current_, sizeof(uint64_t));
                    uint64_t non_ws = ((block + constants::swar8_underscore) | block) & constants::swar8_msb;
                    
                    if (non_ws == 0) {
                        current_ += sizeof(uint64_t);
                    } else {
                        unsigned int byte_idx = std::countr_zero(non_ws) >> 3;
                        current_ += byte_idx;
                        break;
                    }
                }

                // ── Slow Path (Sisa byte & Penyesuaian presisi LookUp table) ──
                while (current_ < end_ && Lookup{}[*current_] == Lookup::Type::WhiteSpace) {
                    ++current_;
                }
                continue;
            }
            case Lookup::Type::LeftCurlyBracket: {
                res_.emplace_back(Token::Type::LeftCurlyBracket);
                hint_.object_count_++;
                current_++;
                continue;
            }
            case Lookup::Type::RightCurlyBracket: {
                res_.emplace_back(Token::Type::RightCurlyBracket);
                current_++;
                continue;
            }
            case Lookup::Type::LeftSquareBracket: {
                res_.emplace_back(Token::Type::LeftSquareBracket);
                hint_.array_count_++;
                current_++;
                continue;
            }
            case Lookup::Type::RightSquareBracket: {
                res_.emplace_back(Token::Type::RightSquareBracket);
                current_++;
                continue;
            }
            case Lookup::Type::Colon: [[likely]] {
                res_.emplace_back(Token::Type::Colon);
                current_++;
                continue;
            }
            case Lookup::Type::Comma: [[likely]] {
                res_.emplace_back(Token::Type::Comma);
                hint_.comma_count_++;
                current_++;
                continue;
            }
            case Lookup::Type::DoubleQuote: [[likely]] {
                readString();
                if (is_error()) {
                    return;
				}
                hint_.string_count_++;
                continue;
            }
            case Lookup::Type::Numeric: {
                readNumeric();
                if (is_error()) {
                    return;
				}
                continue;
            }
            case Lookup::Type::Alphabet: {
                readAlphabet();
                if (is_error()) {
                    return;
				}
                continue;
            }
            case Lookup::Type::SigleQuote: {
                status_ = Error::SingleQuotedString;
                return;
            }
            default: {
                status_ = Error::Unknown;
                return;
            }
        }
    }
    res_.emplace_back(Token::Type::EndOfFile);
}

} // namespace zuu::tokenizer