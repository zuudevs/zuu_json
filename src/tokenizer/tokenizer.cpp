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
#include "utils/strings.hpp"

namespace zuu::tokenizer {

Tokenizer::Tokenizer(std::span<const char> json_content) noexcept
 : current_(json_content.data())
 , end_(json_content.data() + json_content.size()) {
    res_.reserve(json_content.size() / 4);
    tokenize();
}

Tokenizer::Expected Tokenizer::result() noexcept {
    if (is_error()) {
        return std::unexpected{status_};
    }
    return res_;
}

Tokenizer::Expected Tokenizer::Tokenize(Tokenizer::Raw json_content) noexcept {
    auto tokens = Tokenizer(json_content);
    return tokens.result();
}

bool Tokenizer::is_error() const noexcept {
    return status_ != Error::None;
}

void Tokenizer::readString() noexcept {
    auto begin = ++current_;

    while (current_ < end_) {
        if (*current_ == '\\') {
            current_++;
            if (current_ < end_) {
                current_++;
            }
            continue;
        }
        if (*current_ == '\"') {
            break;
        }
        current_++;
    }

    if (current_ >= end_ || *current_ != '\"') {
        status_ = Error::InvalidValue;
        return;
    }

    res_.emplace_back(Token::Type::String, begin, current_ - begin);
    current_++;
}

void Tokenizer::readNumeric() noexcept {
    auto begin = current_;
    auto type = Token::Type::Integer;

    if (current_ < end_ && *current_ == '-') {
        current_++;
    }

    if (current_ < end_ && *current_ == '0') {
        current_++;
        if (current_ < end_ && utils::is_numeric(*current_)) {
            status_ = Error::LeadingZero;
            return;
        }
    } else if (current_ < end_ && utils::is_numeric(*current_)) {
        while (current_ < end_ && utils::is_numeric(*current_)) {
            current_++;
        }
    } else {
        status_ = Error::InvalidValue;
        return;
    }

    if (current_ < end_ && *current_ == '.') {
        type = Token::Type::Double;
        current_++;

        if (current_ >= end_ || !utils::is_numeric(*current_)) {
            status_ = Error::InvalidValue;
            return;
        }

        while (current_ < end_ && utils::is_numeric(*current_)) {
            current_++;
        }
    }

    if (current_ < end_ && (*current_ == 'e' || *current_ == 'E')) {
        type = Token::Type::Double;
        current_++;

        if (current_ < end_ && (*current_ == '+' || *current_ == '-')) {
            current_++;
        }

        if (current_ >= end_ || !utils::is_numeric(*current_)) {
            status_ = Error::InvalidValue;
            return;
        }

        while (current_ < end_ && utils::is_numeric(*current_)) {
            current_++;
        }
    }

    if (!is_error()) {
        res_.emplace_back(type, begin, current_ - begin);
    }
}

void Tokenizer::readAlphabet() noexcept {
    switch (*current_) {
        case 'n': {
			const auto size = sizeof("null") - 1;
            if (current_ + size <= end_ && memcmp(current_ + 1, "ull", size - 1) == 0) {
                res_.emplace_back(Token::Type::Null, current_, size);

                current_ += size;
                return;
            }
            break;
		}
        case 't': {
			const auto size = sizeof("true") - 1;
            if (current_ + size <= end_ && memcmp(current_ + 1, "rue", size - 1) == 0) {
                res_.emplace_back(Token::Type::Boolean, current_, size);

                current_ += size;
                return;
            }
            break;
		}
        case 'f': {
			const auto size = sizeof("false") - 1;
            if (current_ + size <= end_ && memcmp(current_ + 1, "alse", size - 1) == 0) {
                res_.emplace_back(Token::Type::Boolean, current_, size);

                current_ += size;
                return;
            }
            break;
		}
		default:
			status_ = Error::InvalidValue;
    }
}

void Tokenizer::tokenize() noexcept {
    while (current_ < end_) {
        while (current_ < end_ && utils::is_whitespace(*current_)) {
			current_++;
		}

        if (current_ >= end_) {
            break;
        }

        switch (*current_) {
            case '{': {
                res_.emplace_back(Token::Type::LeftCurlyBracket);
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
                continue;
            }
            case '\'': {
                status_ = Error::SingleQuotedString;
                return;
            }
            default: {
                if (utils::is_numeric(*current_) || *current_ == '-') {
                    readNumeric();
                } else if (utils::is_alphabet(*current_)) {
                    readAlphabet();
                } else {
                    status_ = Error::Unknown;
                    return;
                }

                if (is_error()) {
                    return;
                }
            }
        }
    }
}

} // namespace zuu::tokenizer