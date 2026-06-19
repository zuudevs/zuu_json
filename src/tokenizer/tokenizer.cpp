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
#include <cstring>
#include "tokenizer/tokenizer.hpp"
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

    if constexpr (std::endian::native == std::endian::little) {
        while (ptr + constants::byte <= end) {
            unsigned long long block{};
            std::memcpy(&block, ptr, constants::byte);

            unsigned long long match_mask = utils::get_quote_or_escape_mask(block);

            if (match_mask != constants::zero) [[unlikely]] {
                unsigned int byte_idx = std::countr_zero(match_mask) >> 3;

                if (ptr[byte_idx] == '\"') [[likely]] {
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
                    has_escape = true;
                    ptr += byte_idx + 2;
                    continue;
                }
            }
            ptr += constants::byte;
        }
    }

    while (ptr < end) {
        char c = *ptr;
        if (c == '\"') [[likely]] {
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
            if (ptr > end) [[unlikely]] {
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
    auto begin = current_;
    auto type = Token::Type::Integer;

    if (current_ < end_ && *current_ == '-') {
        current_++;
    }

    if (current_ < end_ && *current_ == '0') {
        current_++;
        if (current_ < end_ && (static_cast<unsigned char>(*current_ - '0') < constants::digit)) {
            status_ = Error::LeadingZero;
            return;
        }
    } else if (current_ < end_ &&
               (static_cast<unsigned char>(*current_ - '0') < constants::digit)) {
        while (current_ < end_ &&
               (static_cast<unsigned char>(*current_ - '0') < constants::digit)) {
            current_++;
        }
    } else {
        status_ = Error::InvalidValue;
        return;
    }

    if (current_ < end_ && *current_ == '.') {
        type = Token::Type::Double;
        current_++;

        if (current_ >= end_ || (static_cast<unsigned char>(*current_ - '0') >= constants::digit)) [[unlikely]] {
            status_ = Error::InvalidValue;
            return;
        }

        while (current_ < end_ &&
               (static_cast<unsigned char>(*current_ - '0') < constants::digit)) {
            current_++;
        }
    }

    if (current_ < end_ && (*current_ == 'e' || *current_ == 'E')) {
        type = Token::Type::Double;
        current_++;

        if (current_ < end_ && (*current_ == '+' || *current_ == '-')) {
            current_++;
        }

        if (current_ >= end_ || (static_cast<unsigned char>(*current_ - '0') >= constants::digit)) [[unlikely]] {
            status_ = Error::InvalidValue;
            return;
        }

        while (current_ < end_ &&
               (static_cast<unsigned char>(*current_ - '0') < constants::digit)) {
            current_++;
        }
    }

    if (!is_error()) [[likely]] {
        res_.emplace_back(type, std::string_view(begin, current_ - begin));
    }
}

void Tokenizer::readAlphabet() noexcept {
    const auto rem = end_ - current_;
    
    if (rem >= 4) [[likely]] {
        unsigned val{};
        std::memcpy(&val, current_, 4);

        switch (val) {
            case constants::null_word:
                res_.emplace_back(
					Token::Type::Null, 
					std::string_view(current_, 4)
				);
                current_ += 4;
                return;
                
            case constants::true_word:
                res_.emplace_back(
					Token::Type::Boolean, 
					std::string_view(current_, 4)
				);
                current_ += 4;
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
                break;
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
				if constexpr (std::endian::native == std::endian::little) {
                    while (current_ + constants::byte <= end_) {
                        unsigned long long block{};
                        std::memcpy(&block, current_, constants::byte);
						if (
							block == constants::swar_space_bytes ||
							block == constants::swar_htab_bytes
						) {
							current_ += constants::byte;
						} else {
							break;
						}
                    }
                }
				do {
					++current_;
				} while (current_ < end_ && static_cast<unsigned char>(*current_) <= 0x20);
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
                if (is_error()) [[unlikely]] {
                    return;
				}
                hint_.string_count_++;
                continue;
            }
            case Lookup::Type::Numeric: {
                readNumeric();
                if (is_error()) [[unlikely]] {
                    return;
				}
                continue;
            }
            case Lookup::Type::Alphabet: {
                readAlphabet();
                if (is_error()) [[unlikely]] {
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