/**
 * @file tokenizer_base.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief CRTP Base for Fused Streaming Tokenizer Backends
 * @version 1.5.0
 * @date 2026-06-29
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include <bit>
#include <cstring>
#include <span>

#include "constants/swar.hpp"
#include "enums/token_kind.hpp"
#include "enums/token_type.hpp"
#include "lookups/token.hpp"
#include "models/token.hpp"
#include "utils/compiler.hpp"

#include "zuu_json/core/error.hpp"

namespace zuu::lexer {

/**
 * @brief LexerBase menggunakan Curiously Recurring Template Pattern (CRTP).
 * Bertindak sebagai incremental lexer (streaming) tanpa menyimpan array token.
 */
template <typename Derived>
class LexerBase {
  public:
    using Token = models::Token;
    using Error = core::JsonError;
    using Raw = std::span<const char>;

    explicit LexerBase(Raw json_content) noexcept
        : begin_ptr_(json_content.data())
        , current_(json_content.data())
        , end_(json_content.data() + json_content.size()) {}

    void
        reset() noexcept {
        current_ = begin_ptr_;
        status_ = Error::None;
    }

    [[nodiscard]] ZUU_HOT ZUU_ALWAYS_INLINE Token
        next_token() noexcept {
        while (current_ < end_) {
            auto actionType = static_cast<enums::TokenKind>(lookups::token_kind[*current_]);
            switch (actionType) {
                case enums::TokenKind::WhiteSpace: {
                    derived().skip_whitespace();
                    continue;
                }
                case enums::TokenKind::LeftCurlyBracket: {
                    current_++;
                    return enums::TokenType::LeftCurlyBracket;
                }
                case enums::TokenKind::RightCurlyBracket: {
                    current_++;
                    return enums::TokenType::RightCurlyBracket;
                }
                case enums::TokenKind::LeftSquareBracket: {
                    current_++;
                    return enums::TokenType::LeftSquareBracket;
                }
                case enums::TokenKind::RightSquareBracket: {
                    current_++;
                    return enums::TokenType::RightSquareBracket;
                }
                case enums::TokenKind::Colon:
                    [[likely]] {
                        current_++;
                        return enums::TokenType::Colon;
                    }
                case enums::TokenKind::Comma:
                    [[likely]] {
                        current_++;
                        return enums::TokenType::Comma;
                    }
                case enums::TokenKind::DoubleQuote:
                    [[likely]] {
                        return derived().read_string();
                    }
                case enums::TokenKind::Numeric: {
                    return read_numeric();
                }
                case enums::TokenKind::Alphabet: {
                    return read_alphabet();
                }
                case enums::TokenKind::SigleQuote: {
                    status_ = Error::SingleQuotedString;
                    return enums::TokenType::Unknown;
                }
                default: {
                    status_ = Error::Unknown;
                    return enums::TokenType::Unknown;
                }
            }
        }
        return enums::TokenType::EndOfFile;
    }

    [[nodiscard]] Error
        get_error() const noexcept {
        return status_;
    }

    [[nodiscard]] bool
        is_error() const noexcept {
        return status_ != Error::None;
    }

  protected:
    const char* begin_ptr_;
    const char* current_;
    const char* end_;
    Error status_{Error::None};

    [[nodiscard]] ZUU_HOT ZUU_ALWAYS_INLINE Derived&
        derived() noexcept {
        return *static_cast<Derived*>(this);
    }

    Token
        read_numeric() noexcept {
        const char* begin = current_;
        auto type = enums::TokenType::Integer;
        auto skip_digits = [this]() noexcept {
            while (current_ + sizeof(uint64_t) <= end_) {
                uint64_t block{};
                std::memcpy(&block, current_, sizeof(uint64_t));

                uint64_t val = block - constants::swar8_zero;
                uint64_t non_digits =
                    ((val + constants::swar8_digit_bias) | val) & constants::swar8_msb;

                if (non_digits == constants::zero) {
                    current_ += constants::byte;
                } else {
                    current_ += (std::countr_zero(non_digits) >> 3);
                    return;
                }
            }
            while (current_ < end_ &&
                   static_cast<unsigned char>(*current_ - '0') < constants::digit) {
                current_++;
            }
        };

        if (*current_ == '-') {
            current_++;
            if (current_ == end_) {
                status_ = Error::InvalidValue;
                return enums::TokenType::Unknown;
            }
        }

        if (*current_ == '0') {
            current_++;
            if (current_ < end_ && static_cast<unsigned char>(*current_ - '0') < constants::digit) {
                status_ = Error::LeadingZero;
                return enums::TokenType::Unknown;
            }
        } else if (static_cast<unsigned char>(*current_ - '0') < constants::digit) {
            skip_digits();
        } else {
            status_ = Error::InvalidValue;
            return enums::TokenType::Unknown;
        }

        if (current_ < end_ && *current_ == '.') {
            type = enums::TokenType::Double;
            current_++;

            if (current_ == end_ ||
                static_cast<unsigned char>(*current_ - '0') >= constants::digit) {
                status_ = Error::InvalidValue;
                return enums::TokenType::Unknown;
            }
            skip_digits();
        }

        if (current_ < end_ && (*current_ == 'e' || *current_ == 'E')) {
            type = enums::TokenType::Double;
            current_++;

            if (current_ < end_ && (*current_ == '+' || *current_ == '-')) {
                current_++;
            }

            if (current_ == end_ ||
                static_cast<unsigned char>(*current_ - '0') >= constants::digit) {
                status_ = Error::InvalidValue;
                return enums::TokenType::Unknown;
            }
            skip_digits();
        }

        if (!is_error()) [[likely]] {
            return {type, std::string_view(begin, current_ - begin)};
        }
        return enums::TokenType::Unknown;
    }

    Token
        read_alphabet() noexcept {
        const auto rem = end_ - current_;

        if (rem >= constants::nibble) [[likely]] {
            uint32_t val{};
            std::memcpy(&val, current_, constants::nibble);

            switch (val) {
                case constants::null_word: {
                    auto tok = Token{enums::TokenType::Null,
                                     std::string_view(current_, constants::nibble)};
                    current_ += constants::nibble;
                    return tok;
                }
                case constants::true_word: {
                    auto tok = Token{enums::TokenType::Boolean,
                                     std::string_view(current_, constants::nibble)};
                    current_ += constants::nibble;
                    return tok;
                }
                case constants::fals_word: {
                    if (rem >= 5 && current_[4] == 'e') {
                        auto tok = Token{enums::TokenType::Boolean, std::string_view(current_, 5)};
                        current_ += 5;
                        return tok;
                    }
                }
                default:
                    break;
            }
        }
        status_ = Error::InvalidValue;
        return enums::TokenType::Unknown;
    }

    ZUU_HOT Token
        finish_string_scalar(const char* ptr, const char* begin, bool has_escape) noexcept {
        while (ptr < end_) {
            char c = *ptr;
            if (static_cast<unsigned char>(c) < 0x20) {
                status_ = Error::UnescapedCharacter;
                return enums::TokenType::Unknown;
            }

            if (c == '\"') {
                current_ = ptr + 1;
                return {enums::TokenType::String, std::string_view(begin, ptr - begin), has_escape};
            }
            if (c == '\\') [[unlikely]] {
                has_escape = true;
                ptr += 2;
                if (ptr > end_) {
                    status_ = core::JsonError::InvalidValue;
                    return enums::TokenType::Unknown;
                }
                continue;
            }
            ++ptr;
        }

        status_ = core::JsonError::InvalidValue;
        return enums::TokenType::Unknown;
    }
};

} // namespace zuu::lexer