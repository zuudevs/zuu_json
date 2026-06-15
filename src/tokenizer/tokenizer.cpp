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

namespace {

thread_local static inline zuu::models::Hint<zuu::models::Token> g_hint{};

} // namespace

namespace zuu::tokenizer {

Tokenizer::Tokenizer(std::span<const char> json_content) noexcept
    : current_(json_content.data())
	, end_(json_content.data() + json_content.size()) {

    res_.reserve(json_content.size() / 4);
    tokenize();
}

Tokenizer::Expected Tokenizer::result() && noexcept {
    if (is_error()) {
        return std::unexpected{status_};
    }
    return std::pair{std::move(res_), std::move(g_hint)};
}

Tokenizer::Expected Tokenizer::Tokenize(Raw json_content) noexcept {
    return Tokenizer(json_content).result();
}

bool Tokenizer::is_error() const noexcept {
    return status_ != Error::None;
}

void Tokenizer::readString() noexcept {
    auto begin = ++current_;

	do {
		if (*current_ == '\\') {
            current_++;
            if (current_ != end_) {
                current_++;
            }
            continue;
        }
        if (*current_ == '\"') {
            break;
        }
        current_++;
	} while (current_ != end_);

    if (current_ >= end_ || *current_ != '\"') {
        status_ = Error::InvalidValue;
        return;
    }

    res_.emplace_back(
		Token::Type::String, 
		begin, 
		current_ - begin
	);

    current_++;
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
			begin, 
			current_ - begin
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
					current_, 
					size
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
					current_, 
					size
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
					current_, 
					size
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
				g_hint.object_count_++;
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
				g_hint.array_count_++;
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
				g_hint.string_count_++;
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