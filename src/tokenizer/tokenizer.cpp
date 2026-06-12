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

Tokenizer::Tokenizer(std::span<const char> json_content) noexcept
    : raw_(json_content) {

    res_.reserve(json_content.size() / 4);
    tokenize();
}

Tokenizer::Expected Tokenizer::result() && noexcept {
    if (is_error()) {
        return std::unexpected{status_};
    }
    return std::move(res_);
}

Tokenizer::Expected Tokenizer::Tokenize(Raw json_content) noexcept {
    return Tokenizer(json_content).result();
}

bool Tokenizer::is_error() const noexcept {
    return status_ != Error::None;
}

void Tokenizer::readString() noexcept {
    idx_++;
    size_t start = idx_;

    while (idx_ < raw_.size()) {
        if (raw_[idx_] == '\\') {
            idx_++;
            if (idx_ < raw_.size()) {
                idx_++;
            }
            continue;
        }
        if (raw_[idx_] == '\"') {
            break;
        }
        idx_++;
    }

    if (idx_ >= raw_.size() || raw_[idx_] != '\"') {
        status_ = Error::InvalidValue;
        return;
    }

    res_.emplace_back(Token::Type::String, raw_.data() + start, idx_ - start);

    idx_++;
}

void Tokenizer::readNumeric() noexcept {
    size_t start = idx_;
    auto type = Token::Type::Integer;

    if (idx_ < raw_.size() && raw_[idx_] == '-') {
        idx_++;
    }

    if (idx_ < raw_.size() && raw_[idx_] == '0') {
        idx_++;
        if (idx_ < raw_.size() &&
            (static_cast<unsigned char>(raw_[idx_] - '0') < constants::digit)) {
            status_ = Error::LeadingZero;
            return;
        }
    } else if (idx_ < raw_.size() &&
               (static_cast<unsigned char>(raw_[idx_] - '0') < constants::digit)) {
        while (idx_ < raw_.size() &&
               (static_cast<unsigned char>(raw_[idx_] - '0') < constants::digit)) {
            idx_++;
        }
    } else {
        status_ = Error::InvalidValue;
        return;
    }

    if (idx_ < raw_.size() && raw_[idx_] == '.') {
        type = Token::Type::Double;
        idx_++;

        if (idx_ >= raw_.size() ||
            (static_cast<unsigned char>(raw_[idx_] - '0') >= constants::digit)) {
            status_ = Error::InvalidValue;
            return;
        }

        while (idx_ < raw_.size() &&
               (static_cast<unsigned char>(raw_[idx_] - '0') < constants::digit)) {
            idx_++;
        }
    }

    if (idx_ < raw_.size() && (raw_[idx_] == 'e' || raw_[idx_] == 'E')) {
        type = Token::Type::Double;
        idx_++;

        if (idx_ < raw_.size() && (raw_[idx_] == '+' || raw_[idx_] == '-')) {
            idx_++;
        }

        if (idx_ >= raw_.size() ||
            (static_cast<unsigned char>(raw_[idx_] - '0') >= constants::digit)) {
            status_ = Error::InvalidValue;
            return;
        }

        while (idx_ < raw_.size() &&
               (static_cast<unsigned char>(raw_[idx_] - '0') < constants::digit)) {
            idx_++;
        }
    }

    if (!is_error()) {
        res_.emplace_back(type, raw_.data() + start, idx_ - start);
    }
}

void Tokenizer::readAlphabet() noexcept {
    const auto rem = raw_.size() - idx_;
    switch (raw_[idx_]) {
        case 'n': {
            const auto size = sizeof("null") - 1;
            if (rem >= size && raw_[idx_ + 1] == 'u' && raw_[idx_ + 2] == 'l' &&
                raw_[idx_ + 3] == 'l') {
                res_.emplace_back(Token::Type::Null, raw_.data() + idx_, size);
                idx_ += size;
                return;
            }
            break;
        }
        case 't': {
            const auto size = sizeof("true") - 1;
            if (rem >= size && raw_[idx_ + 1] == 'r' && raw_[idx_ + 2] == 'u' &&
                raw_[idx_ + 3] == 'e') {
                res_.emplace_back(Token::Type::Boolean, raw_.data() + idx_, size);
                idx_ += size;
                return;
            }
            break;
        }
        case 'f': {
            const auto size = sizeof("false") - 1;
            if (rem >= size && raw_[idx_ + 1] == 'a' && raw_[idx_ + 2] == 'l' &&
                raw_[idx_ + 3] == 's' && raw_[idx_ + 4] == 'e') {
                res_.emplace_back(Token::Type::Boolean, raw_.data() + idx_, size);
                idx_ += size;
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
    while (idx_ < raw_.size()) {
        char c = raw_[idx_];

        switch (c) {
			case '\t':
			case '\n':
			case '\v':
			case '\f':
			case '\r': 
			case ' ': {
				idx_++;
				continue;
			}
            case '{': {
                res_.emplace_back(Token::Type::LeftCurlyBracket);
                idx_++;
                continue;
            }
            case '}': {
                res_.emplace_back(Token::Type::RightCurlyBracket);
                idx_++;
                continue;
            }
            case '[': {
                res_.emplace_back(Token::Type::LeftSquareBracket);
                idx_++;
                continue;
            }
            case ']': {
                res_.emplace_back(Token::Type::RightSquareBracket);
                idx_++;
                continue;
            }
            case ':': {
                res_.emplace_back(Token::Type::Colon);
                idx_++;
                continue;
            }
            case ',': {
                res_.emplace_back(Token::Type::Comma);
                idx_++;
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