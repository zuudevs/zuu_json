/**
 * @file tokenizer_base.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief CRTP Base for Fused Streaming Tokenizer Backends
 * @version 1.2.0
 * @date 2026-06-29
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "models/token.hpp"
#include "zuu_json/core/error.hpp"
#include "constants/general.hpp"
#include "utils/compiler.hpp"
#include <span>
#include <cstring>
#include <bit>

namespace zuu::tokenizer {

/**
 * @brief TokenizerBase menggunakan Curiously Recurring Template Pattern (CRTP).
 * Bertindak sebagai incremental lexer (streaming) tanpa menyimpan array token.
 */
template <typename Derived>
class TokenizerBase {
  public:
    using Token = models::Token;
    using Lookup = traits::LookupTrait<Token>;
    using Error = core::JsonError;
    using Hint = traits::HintTrait<Token>;
    using Raw = std::span<const char>;

    static constexpr int32_t kMaxDepth = 512;

    explicit TokenizerBase(Raw json_content) noexcept
        : begin_ptr_(json_content.data())
        , current_(json_content.data())
        , end_(json_content.data() + json_content.size()) {}

    [[nodiscard]] Hint pre_scan() noexcept {
        Hint hint{};
        int32_t depth = 0;
        const char* ptr = begin_ptr_;
        while (ptr < end_) {
            switch (*ptr) {
                case '{': 
                    hint.object_count_++; 
                    if (++depth > kMaxDepth) { status_ = Error::DepthLimitExceeded; return hint; }
                    break;
                case '[': 
                    hint.array_count_++; 
                    if (++depth > kMaxDepth) { status_ = Error::DepthLimitExceeded; return hint; }
                    break;
                case '}': case ']':
                    depth--; 
                    break;
                case ',': hint.comma_count_++; break;
                case '"': {
                    hint.string_count_++;
                    const char* start = ptr;
                    bool has_escape = false;
                    ++ptr;
                    while (ptr < end_) {
                        if (*ptr == '"') break;
                        if (*ptr == '\\') {
                            has_escape = true;
                            ptr += 2;
                            continue;
                        }
                        ptr++;
                    }
                    if (has_escape) {
                        hint.string_escape_bytes_ += (ptr - start);
                    }
                    break;
                }
                default: break;
            }
            ptr++;
        }
        reset();
        return hint;
    }

    void reset() noexcept {
        current_ = begin_ptr_;
        status_ = Error::None;
    }

    [[nodiscard]] ZUU_HOT ZUU_ALWAYS_INLINE Token next_token() noexcept {
        while (current_ < end_) {
            auto actionType = Lookup{}[*current_];
            switch (actionType) {
                case Lookup::Type::WhiteSpace:
                    derived().skip_whitespace();
                    continue;
                case Lookup::Type::LeftCurlyBracket:
                    current_++;
                    return Token(Token::Type::LeftCurlyBracket);
                case Lookup::Type::RightCurlyBracket:
                    current_++;
                    return Token(Token::Type::RightCurlyBracket);
                case Lookup::Type::LeftSquareBracket:
                    current_++;
                    return Token(Token::Type::LeftSquareBracket);
                case Lookup::Type::RightSquareBracket:
                    current_++;
                    return Token(Token::Type::RightSquareBracket);
                case Lookup::Type::Colon: [[likely]]
                    current_++;
                    return Token(Token::Type::Colon);
                case Lookup::Type::Comma: [[likely]]
                    current_++;
                    return Token(Token::Type::Comma);
                case Lookup::Type::DoubleQuote: [[likely]]
                    return derived().read_string();
                case Lookup::Type::Numeric:
                    return read_numeric();
                case Lookup::Type::Alphabet:
                    return read_alphabet();
                case Lookup::Type::SigleQuote:
                    status_ = Error::SingleQuotedString;
                    return Token(Token::Type::Unknown);
                default:
                    status_ = Error::Unknown;
                    return Token(Token::Type::Unknown);
            }
        }
        return Token(Token::Type::EndOfFile);
    }

    [[nodiscard]] Error get_error() const noexcept { return status_; }

    [[nodiscard]] bool is_error() const noexcept {
        return status_ != Error::None;
    }

  protected:
    const char* begin_ptr_;
    const char* current_;
    const char* end_;
    Error status_{Error::None};

    [[nodiscard]] ZUU_HOT ZUU_ALWAYS_INLINE Derived& derived() noexcept {
        return *static_cast<Derived*>(this);
    }

    Token read_numeric() noexcept {
        const char* begin = current_;
        auto type = Token::Type::Integer;
        auto skip_digits = [this]() noexcept {
            while (current_ + sizeof(uint64_t) <= end_) {
                uint64_t block{};
                std::memcpy(&block, current_, sizeof(uint64_t));
                
                uint64_t val = block - constants::swar8_zero;
                uint64_t non_digits = ((val + constants::swar8_digit_bias) | val) & constants::swar8_msb;
                
                if (non_digits == 0) {
                    current_ += sizeof(uint64_t);
                } else {
                    current_ += (std::countr_zero(non_digits) >> 3);
                    return;
                }
            }
            // Tail / Fallback untuk sisa karakter (kurang dari 8 byte)
            while (current_ < end_ && static_cast<unsigned char>(*current_ - '0') < constants::digit) {
                current_++;
            }
        };

        if (*current_ == '-') {
            current_++;
            if (current_ == end_) { 
                status_ = Error::InvalidValue; 
                return Token(Token::Type::Unknown); 
            }
        }

        if (*current_ == '0') {
            current_++;
            if (current_ < end_ && static_cast<unsigned char>(*current_ - '0') < constants::digit) {
                status_ = Error::LeadingZero;
                return Token(Token::Type::Unknown);
            }
        } else if (static_cast<unsigned char>(*current_ - '0') < constants::digit) {
            skip_digits();
        } else {
            status_ = Error::InvalidValue;
            return Token(Token::Type::Unknown);
        }

        if (current_ < end_ && *current_ == '.') {
            type = Token::Type::Double;
            current_++;
            
            if (current_ == end_ || static_cast<unsigned char>(*current_ - '0') >= constants::digit) {
                status_ = Error::InvalidValue;
                return Token(Token::Type::Unknown);
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
                return Token(Token::Type::Unknown);
            }
            skip_digits();
        }

        if (!is_error()) [[likely]] {
            return Token(type, std::string_view(begin, current_ - begin));
        }
        return Token(Token::Type::Unknown);
    }

    Token read_alphabet() noexcept {
        const auto rem = end_ - current_;
        
        if (rem >= constants::nibble) [[likely]] {
            uint32_t val{};
            std::memcpy(&val, current_, constants::nibble);

            switch (val) {
                case constants::null_word: {
                    auto tok = Token(Token::Type::Null, std::string_view(current_, constants::nibble));
                    current_ += constants::nibble;
                    return tok;
                }
                case constants::true_word: {
                    auto tok = Token(Token::Type::Boolean, std::string_view(current_, constants::nibble));
                    current_ += constants::nibble;
                    return tok;
                }
                case constants::fals_word: {
                    if (rem >= 5 && current_[4] == 'e') {
                        auto tok = Token(Token::Type::Boolean, std::string_view(current_, 5));
                        current_ += 5;
                        return tok;
                    }
                }
                default:
                    break;
            }
        }
        status_ = Error::InvalidValue;
        return Token(Token::Type::Unknown);
    }

    ZUU_HOT Token finish_string_scalar(const char* ptr, const char* begin, bool has_escape) noexcept {
        while (ptr < end_) {
            char c = *ptr;
            if (static_cast<unsigned char>(c) < 0x20) {
                status_ = Error::UnescapedCharacter;
                return Token(Token::Type::Unknown);
            }

            if (c == '\"') {
                current_ = ptr + 1;
                return Token(Token::Type::String, std::string_view(begin, ptr - begin), has_escape);
            }
            if (c == '\\') [[unlikely]] {
                has_escape = true;
                ptr += 2;
                if (ptr > end_) {
                    status_ = core::JsonError::InvalidValue;
                    return Token(Token::Type::Unknown);
                }
                continue;
            }
            ++ptr;
        }

        status_ = core::JsonError::InvalidValue;
        return Token(Token::Type::Unknown);
    }
};

} // namespace zuu::tokenizer