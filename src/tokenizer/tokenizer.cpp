/**
 * @file tokenizer.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-06
 *
 * @copyright Copyright (c) 2026
 */

#include "tokenizer/tokenizer.hpp"
#include "constants/general.hpp"
#include "utils/strings.hpp"

namespace zuu::tokenizer {

[[nodiscard]] inline constexpr bool has_zero_byte(uint64_t v) noexcept {
    return (v - 0x0101010101010101ULL) & ~v & 0x8080808080808080ULL;
}

[[nodiscard]] inline constexpr bool has_quote_or_escape(uint64_t v) noexcept {
    const uint64_t quote_mask = v ^ 0x2222222222222222ULL;
    const uint64_t escape_mask = v ^ 0x5C5C5C5C5C5C5C5CULL;
    return has_zero_byte(quote_mask) | has_zero_byte(escape_mask);
}

Tokenizer::Tokenizer(std::span<const char> json_content) noexcept
    : current_(json_content.data())
	, end_(json_content.data() + json_content.size()) {

    res_.reserve((json_content.size() >> 1) + 16);
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

    // Fast-path SWAR: Pindai 8 bytes sekaligus
    while (ptr + 8 <= end) {
        uint64_t block{};
        std::memcpy(&block, ptr, 8);

        if (has_quote_or_escape(block)) {
            break; // Jika ada '"' atau '\', keluar dan evaluasi secara presisi di slow-path
        }
        ptr += 8;
    }

    // Slow-path skalar untuk resolusi akhir dan tracking escape character
    while (ptr < end) {
        char c = *ptr;
        if (c == '"') [[likely]] {
            res_.emplace_back(
                Token::Type::String, 
				std::string_view(begin, ptr - begin), 
				has_escape
			);

            // Catat kebutuhan buffer jika string memiliki escape
            if (has_escape) {
                hint_.string_escape_bytes_ += (ptr - begin);
            }

            current_ = ptr + 1;
            return;
        }
        if (c == '\\') [[unlikely]] {
            has_escape = true;
            ptr += 2; // Lewati karakter escape
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
    auto begin = current_;
    auto type = Token::Type::Integer;

    if (
		current_ < end_ && 
		*current_ == '-'
	) {
        current_++;
    }

    if (
		current_ < end_ && 
		*current_ == '0'
	) {
        current_++;
        if (
			current_ < end_ &&
            (static_cast<unsigned char>(*current_ - '0') < constants::digit)
		) {
            status_ = Error::LeadingZero;
            return;
        }
    } else if (
		current_ < end_ && 
		(static_cast<unsigned char>(*current_ - '0') < constants::digit)
	) {
        while (
			current_ < end_ && 
			(static_cast<unsigned char>(*current_ - '0') < constants::digit)
		) {
            current_++;
        }
    } else {
        status_ = Error::InvalidValue;
        return;
    }

    if (
		current_ < end_ && 
		*current_ == '.'
	) {
        type = Token::Type::Double;
        current_++;

        if (
			current_ >= end_ || 
			(static_cast<unsigned char>(*current_ - '0') >= constants::digit)
		) {
            status_ = Error::InvalidValue;
            return;
        }

        while (
			current_ < end_ && 
			(static_cast<unsigned char>(*current_ - '0') < constants::digit)
		) {
            current_++;
        }
    }

    if (
		current_ < end_ && 
		(*current_ == 'e' || *current_ == 'E')
	) {
        type = Token::Type::Double;
        current_++;

        if (
			current_ < end_ && 
			(*current_ == '+' || *current_ == '-')
		) {
            current_++;
        }

        if (
			current_ >= end_ ||
            (static_cast<unsigned char>(*current_ - '0') >= constants::digit)
		) {
            status_ = Error::InvalidValue;
            return;
        }

        while (
			current_ < end_ && 
			(static_cast<unsigned char>(*current_ - '0') < constants::digit)
		) {
            current_++;
        }
    }

    if (!is_error()) {
        res_.emplace_back(
			type, 
			std::string_view(begin, current_ - begin)
		);
    }
}

void Tokenizer::readAlphabet() noexcept {
    const auto rem = end_ - current_;
    switch (*current_) {
        case 'n': {
            const auto size = sizeof("null") - 1;
            if (
				rem >= size && 
				*(current_ + 1) == 'u' && 
				*(current_ + 2) == 'l' &&
                *(current_ + 3) == 'l'
			) {
                res_.emplace_back(
					Token::Type::Null, 
					std::string_view(current_, size)
				);
                current_ += size;
                return;
            }
            break;
        }
        case 't': {
            const auto size = sizeof("true") - 1;
            if (
				rem >= size && 
				*(current_ + 1) == 'r' && 
				*(current_ + 2) == 'u' &&
                *(current_ + 3) == 'e'
			) {
                res_.emplace_back(
					Token::Type::Boolean, 
					std::string_view(current_, size)
				);
                current_ += size;
                return;
            }
            break;
        }
        case 'f': {
            const auto size = sizeof("false") - 1;
            if (
				rem >= size && 
				*(current_ + 1) == 'a' && 
				*(current_ + 2) == 'l' &&
                *(current_ + 3) == 's' && 
				*(current_ + 4) == 'e'
			) {
                res_.emplace_back(
					Token::Type::Boolean, 
					std::string_view(current_, size)
				);
                current_ += size;
                return;
            }
            break;
        }
        default: {
            status_ = Error::InvalidValue;
        }
    }
}

void Tokenizer::tokenize() noexcept {
    while (current_ < end_) {
		char c = *current_;
        switch (c) {
			case '\t':
			case '\n':
			case '\v':
			case '\f':
			case '\r': 
			case ' ': {
				current_++;
				continue;
			}
            case '{': {
                res_.emplace_back(Token::Type::LeftCurlyBracket);
				hint_.object_count_++;
                current_++;
                continue;
            }
            case '}': {
                res_.emplace_back(Token::Type::RightCurlyBracket);
                current_++;
                continue;
            }
            case '[': {
                res_.emplace_back(Token::Type::LeftSquareBracket);
				hint_.array_count_++;
                current_++;
                continue;
            }
            case ']': {
                res_.emplace_back(Token::Type::RightSquareBracket);
                current_++;
                continue;
            }
            case ':': {
                res_.emplace_back(Token::Type::Colon);
                current_++;
                continue;
            }
            case ',': {
                res_.emplace_back(Token::Type::Comma);
                current_++;
                continue;
            }
            case '\"': {
                readString();
                if (is_error())
                    return;
				hint_.string_count_++;
                continue;
            }
            case '\'': {
                status_ = Error::SingleQuotedString;
                return;
            }
			default: {
				if (utils::is_numeric(c) || c == '-') {
					readNumeric();
				} else if (utils::is_alphabet(c)) {
					readAlphabet();
				} else {
					status_ = Error::Unknown;
					return;
				}

				if (is_error())
					return;
			}
        }
    }
	res_.emplace_back(Token::Type::EndOfFile);
}

} // namespace zuu::tokenizer