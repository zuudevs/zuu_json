/**
 * @file tokenizer_base.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief CRTP Base for Tokenizer Backends
 * @version 1.1.1
 * @date 2026-06-26
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "models/token.hpp"
#include "zuu_json/core/error.hpp"
#include "constants/general.hpp"
#include "utils/compiler.hpp"
#include <bit>
#include <expected>
#include <span>
#include <vector>
#include <cstring>

namespace zuu::tokenizer {

/**
 * @brief TokenizerBase menggunakan Curiously Recurring Template Pattern (CRTP).
 * Ini memastikan delegasi fungsi ke `Derived` terjadi saat compile-time (Zero-Cost).
 */
template <typename Derived>
class TokenizerBase {
  public:
    using Token = models::Token;
    using Lookup = traits::LookupTrait<Token>;
    using Error = core::JsonError;
    using Result = std::vector<Token>;
    using Hint = traits::HintTrait<Token>;
    using Resource = std::pair<Result, Hint>;
    using Expected = std::expected<Resource, Error>;
    using Raw = std::span<const char>;

    explicit TokenizerBase(Raw json_content) noexcept
        : current_(json_content.data())
        , end_(json_content.data() + json_content.size()) {
        
        res_.reserve((json_content.size() >> 1) + constants::word);
    }

    [[nodiscard]] Expected result() && noexcept {
        if (is_error()) {
            return std::unexpected{status_};
        }
        return std::pair{std::move(res_), hint_};
    }

    void execute() noexcept {
        tokenize_loop();
    }

  protected:
    Result res_;
    Hint hint_{};
    const char* current_;
    const char* end_;
    Error status_{Error::None};

    [[nodiscard]] ZUU_HOT ZUU_ALWAYS_INLINE Derived& derived() noexcept {
        return *static_cast<Derived*>(this);
    }

    [[nodiscard]] bool is_error() const noexcept {
        return status_ != Error::None;
    }

    void read_numeric() noexcept {
        const char* begin = current_;
        auto type = Token::Type::Integer;

        // Abstraksi yang bersih: skip_digits cukup menggunakan scalar loop,
        // karena lompatan digit umumnya sangat pendek (1-5 karakter) sehingga 
        // setup SIMD/SWAR malah menimbulkan overhead.
        auto skip_digits = [this]() noexcept {
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

    void read_alphabet() noexcept {
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

    // Scalar fallback ketika SIMD/SWAR menemukan unescaped character atau mencapai batas blok
    ZUU_HOT void finish_string_scalar(const char* ptr, const char* begin, bool has_escape) noexcept {
        while (ptr < end_) {
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
                if (ptr > end_) {
                    status_ = core::JsonError::InvalidValue;
                    return;
                }
                continue;
            }
            ++ptr;
        }

        status_ = core::JsonError::InvalidValue;
    }

    void tokenize_loop() noexcept {
        while (current_ < end_) {
            auto actionType = Lookup{}[*current_];
            switch (actionType) {
                case Lookup::Type::WhiteSpace: {
                    derived().skip_whitespace();
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
                    derived().read_string();
                    if (is_error()) return;
                    hint_.string_count_++;
                    continue;
                }
                case Lookup::Type::Numeric: {
                    read_numeric();
                    if (is_error()) return;
                    continue;
                }
                case Lookup::Type::Alphabet: {
                    read_alphabet();
                    if (is_error()) return;
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
};

} // namespace zuu::tokenizer