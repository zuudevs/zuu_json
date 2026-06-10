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

#ifdef CPP_JSON_DEBUG
    #include <format>
    #include <print>

    #include "utils/filesystem.hpp"
    #include "utils/timer.hpp"

static inline zuu::utils::Timer __timer__;
static inline std::string __message__;

    #define START_CAPTURE()                                                                        \
        do {                                                                                       \
            __timer__.start();                                                                     \
        } while (false)

    #define END_CAPTURE(subject, filepath)                                                         \
        do {                                                                                       \
            __message__ = std::format("{}: {}", subject, __timer__.duration());                    \
                                                                                                   \
            auto __result__ = zuu::utils::FileSystem::WriteFile(                                   \
                filepath, std::span<const char>(__message__.data(), __message__.size()), true);    \
                                                                                                   \
            if (!__result__) {                                                                     \
                std::print(stderr, "Error: {}\n", static_cast<int>(__result__.error()));           \
            }                                                                                      \
        } while (false)

#else

    #define START_CAPTURE() ((void)0)
    #define END_CAPTURE(subject, filepath) ((void)0)

#endif

namespace zuu::tokenizer {

Tokenizer::Tokenizer(std::span<const char> json_content) noexcept
    : raw_(json_content) {

    START_CAPTURE();

    res_.reserve(json_content.size() / 4);

    END_CAPTURE("tokenizer construction time", "result/report.txt");

    tokenize();
}

Tokenizer::Expected Tokenizer::result() && noexcept {
    if (is_error()) {
        return std::unexpected{status_};
    }
    return std::move(res_);
}

Tokenizer::Expected Tokenizer::Tokenize(Raw json_content) noexcept {
    auto tokens = Tokenizer(json_content);
    return std::move(tokens).result();
}

bool Tokenizer::is_error() const noexcept {
    return status_ != Error::None;
}

void Tokenizer::advance() noexcept {
    idx_++;
}

void Tokenizer::skip_whitespace() noexcept {
    while (idx_ < raw_.size() && utils::is_whitespace(raw_[idx_])) {
        advance();
    }
}

void Tokenizer::readString() noexcept {
    advance();
    size_t start = idx_;

    while (idx_ < raw_.size()) {
        if (raw_[idx_] == '\\') {
            advance();
            if (idx_ < raw_.size()) {
                advance();
            }
            continue;
        }
        if (raw_[idx_] == '\"') {
            break;
        }
        advance();
    }

    if (idx_ >= raw_.size() || raw_[idx_] != '\"') {
        status_ = Error::InvalidValue;
        return;
    }

    res_.emplace_back(Token::Type::String, raw_.data() + start, idx_ - start);

    advance();
}

void Tokenizer::readNumeric() noexcept {
    size_t start = idx_;
    auto type = Token::Type::Integer;

    if (idx_ < raw_.size() && raw_[idx_] == '-') {
        advance();
    }

    if (idx_ < raw_.size() && raw_[idx_] == '0') {
        advance();
        if (idx_ < raw_.size() && utils::is_numeric(raw_[idx_])) {
            status_ = Error::LeadingZero;
            return;
        }
    } else if (idx_ < raw_.size() && utils::is_numeric(raw_[idx_])) {
        while (idx_ < raw_.size() && utils::is_numeric(raw_[idx_])) {
            advance();
        }
    } else {
        status_ = Error::InvalidValue;
        return;
    }

    if (idx_ < raw_.size() && raw_[idx_] == '.') {
        type = Token::Type::Double;
        advance();

        if (idx_ >= raw_.size() || (static_cast<unsigned char>(raw_[idx_]) - '0' >= constants::digit)) {
            status_ = Error::InvalidValue;
            return;
        }

        while (idx_ < raw_.size() && (static_cast<unsigned char>(raw_[idx_]) - '0' < constants::digit)) {
            advance();
        }
    }

    if (idx_ < raw_.size() && (raw_[idx_] == 'e' || raw_[idx_] == 'E')) {
        type = Token::Type::Double;
        advance();

        if (idx_ < raw_.size() && (raw_[idx_] == '+' || raw_[idx_] == '-')) {
            advance();
        }

        if (idx_ >= raw_.size() || (static_cast<unsigned char>(raw_[idx_]) - '0' >= constants::digit)) {
            status_ = Error::InvalidValue;
            return;
        }

        while (idx_ < raw_.size() && (static_cast<unsigned char>(raw_[idx_]) - '0' < constants::digit)) {
            advance();
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
            break;
        }
    }

    status_ = Error::InvalidValue;
}

void Tokenizer::tokenize() noexcept {
    while (idx_ < raw_.size()) {
        skip_whitespace();

        if (idx_ >= raw_.size()) {
            break;
        }

        char c = raw_[idx_];

        switch (c) {
            case '{': {
                res_.emplace_back(Token::Type::LeftCurlyBracket);
                advance();
                continue;
            }
            case '}': {
                res_.emplace_back(Token::Type::RightCurlyBracket);
                advance();
                continue;
            }
            case '[': {
                res_.emplace_back(Token::Type::LeftSquareBracket);
                advance();
                continue;
            }
            case ']': {
                res_.emplace_back(Token::Type::RightSquareBracket);
                advance();
                continue;
            }
            case ':': {
                res_.emplace_back(Token::Type::Colon);
                advance();
                continue;
            }
            case ',': {
                res_.emplace_back(Token::Type::Comma);
                advance();
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
        }

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

} // namespace zuu::tokenizer