/**
 * @file parser_base.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief CRTP Base for Parser Backends
 * @version 1.1.0
 * @date 2026-06-27
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "constants/general.hpp"
#include "models/depth_guard.hpp"
#include "models/storage.hpp"
#include "utils/parser.hpp"
#include "utils/strings.hpp"
#include "zuu_json/core/error.hpp"
#include <expected>
#include <span>
#include <cstring>
#include "utils/compiler.hpp"

namespace zuu::parser {

/**
 * @brief ParserBase menggunakan Curiously Recurring Template Pattern (CRTP).
 * Delegasi metode polimorfik di-_resolve_ saat compile-time.
 */
template <typename Derived>
class ParserBase {
  public:
    using Token = models::Token;
    using Hint = traits::HintTrait<Token>;
    using TokenType = Token::Type;
    using Error = core::JsonError;
    using Storage = models::Storage;
    using JsonValue = models::JsonValue;
    using JsonMember = models::JsonMember;
    using Raw = std::span<const Token>;
    using Expected = std::expected<Storage, Error>;

    explicit ParserBase(Raw tokens, Hint hint) noexcept
        : current_(tokens.data()), end_(tokens.data() + tokens.size()), res_(hint) {}

    [[nodiscard]] Expected result() && noexcept {
        if (has_error()) {
            return std::unexpected{status_};
        }
        return std::move(res_);
    }

    [[nodiscard]] bool has_error() const noexcept {
        return status_ != Error::None;
    }

    void execute() noexcept {
        derived().parse();
    }

  protected:
    Storage res_;
    const Token* current_;
    const Token* end_;
    uint32_t current_depth_{0};
    Error status_{core::JsonError::None};

    static constexpr uint32_t kMaxDepth = 512;

    [[nodiscard]] inline Derived& derived() noexcept {
        return *static_cast<Derived*>(this);
    }

    [[nodiscard]] uint32_t decodeUnicodeHex(const char* ptr) noexcept {
        uint32_t d0 = traits::LookupTrait<char>::hex.datas_[static_cast<uint8_t>(ptr[0])];
        uint32_t d1 = traits::LookupTrait<char>::hex.datas_[static_cast<uint8_t>(ptr[1])];
        uint32_t d2 = traits::LookupTrait<char>::hex.datas_[static_cast<uint8_t>(ptr[2])];
        uint32_t d3 = traits::LookupTrait<char>::hex.datas_[static_cast<uint8_t>(ptr[3])];

        if ((d0 | d1 | d2 | d3) > constants::hex_alpha_max_val) [[unlikely]] {
            status_ = core::JsonError::InvalidValue;
            return constants::zero;
        }

        return (d0 << 12) | (d1 << constants::byte) | (d2 << constants::nibble) | d3;
    }

    ZUU_HOT void finish_string_scalar(char*& out, const char*& ptr, const char* end) noexcept {
        while (ptr < end) {
            while (ptr < end && *ptr != '\\') {
                *out++ = *ptr++;
            }

            if (ptr >= end) {
                break;
            }

            ++ptr; 
            if (ptr >= end) {
                break;
            }

            switch (*ptr) {
                case '\"':  *out++ = '"';  break;
                case '\\': *out++ = '\\'; break;
                case '/':  *out++ = '/';  break;
                case 'b':  *out++ = '\b'; break;
                case 'f':  *out++ = '\f'; break;
                case 'n':  *out++ = '\n'; break;
                case 'r':  *out++ = '\r'; break;
                case 't':  *out++ = '\t'; break;
                case 'u': {
                    if (ptr + 5 > end) {
                        status_ = core::JsonError::InvalidValue;
                        return;
                    }
                    
                    uint32_t cp = decodeUnicodeHex(ptr + 1);
                    ptr += constants::nibble;
                    if (cp >= 0xD800 && cp <= 0xDBFF) {
                        if (ptr + 6 <= end && ptr[1] == '\\' && ptr[2] == 'u') {
                            uint32_t cp2 = decodeUnicodeHex(ptr + 3);
                            if (cp2 >= 0xDC00 && cp2 <= 0xDFFF) {
                                cp = 0x10000 + (((cp - 0xD800) << constants::digit) | (cp2 - 0xDC00));
                                ptr += 6;
                            } else {
                                status_ = core::JsonError::InvalidValue;
                                return;
                            }
                        } else {
                            status_ = core::JsonError::InvalidValue;
                            return;
                        }
                    }

                    if (cp <= 0x7F) {
                        *out++ = static_cast<char>(cp);
                    } else if (cp <= 0x7FF) {
                        *out++ = static_cast<char>(0xC0 | ((cp >> 6) & 0x1F));
                        *out++ = static_cast<char>(0x80 | (cp & 0x3F));
                    } else if (cp <= 0xFFFF) {
                        *out++ = static_cast<char>(0xE0 | ((cp >> 12) & 0x0F));
                        *out++ = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                        *out++ = static_cast<char>(0x80 | (cp & 0x3F));
                    } else if (cp <= 0x10FFFF) {
                        *out++ = static_cast<char>(0xF0 | ((cp >> 18) & 0x07));
                        *out++ = static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
                        *out++ = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                        *out++ = static_cast<char>(0x80 | (cp & 0x3F));
                    } else {
                        status_ = core::JsonError::InvalidValue;
                        return;
                    }
                    break;
                }
                default:
                    status_ = core::JsonError::InvalidValue;
                    return;
            }
            ++ptr;
        }
    }

    [[nodiscard]] JsonValue buildNull() noexcept {
        current_++;
        return JsonValue::Null();
    }

    [[nodiscard]] JsonValue buildBoolean() noexcept {
        const auto value = current_->begin_[0] == 't';
        current_++;
        return JsonValue::Boolean(value);
    }

    [[nodiscard]] JsonValue buildInteger() noexcept {
        if (auto res = utils::parse<long long>(current_->begin_, current_->begin_ + current_->size_); res) {
            current_++;
            return JsonValue::Integer(*res);
        } else {
            status_ = core::JsonError::InvalidValue;
            return JsonValue::Null();
        }
    }

    [[nodiscard]] JsonValue buildDouble() noexcept {
        if (auto res = utils::parse<double>(current_->begin_, current_->begin_ + current_->size_); res) {
            current_++;
            return JsonValue::Double(*res);
        } else {
            status_ = core::JsonError::InvalidValue;
            return JsonValue::Null();
        }
    }

    [[nodiscard]] JsonValue buildString() noexcept {
        std::string_view val = current_->value();

        if (current_->has_escape_) {
            val = derived().unescapeString(val);
            if (has_error()) {
                return JsonValue::Null();
            }
        }

        const auto index = res_.commitString(val);
        ++current_;
        return JsonValue::String(index);
    }

    [[nodiscard]] JsonValue buildArray() noexcept {
        if (current_depth_ >= kMaxDepth) [[unlikely]] {
            status_ = core::JsonError::DepthLimitExceeded;
            return JsonValue::Null();
        }
        models::DepthGuard guard(current_depth_);

        ++current_;

        if (current_->type_ == TokenType::RightSquareBracket) {
            ++current_;
            return JsonValue::Array(res_.sealArray(res_.getArrayOffset()));
        }

        const uint64_t start_offset = res_.getArrayOffset();

        while (true) {
            JsonValue value;
            switch (current_->type_) {
                case TokenType::String:            value = derived().buildString(); break;
                case TokenType::Integer:           value = derived().buildInteger(); break;
                case TokenType::Boolean:           value = derived().buildBoolean(); break;
                case TokenType::Double:            value = derived().buildDouble(); break;
                case TokenType::Null:              value = derived().buildNull(); break;
                case TokenType::LeftCurlyBracket:  value = derived().buildObject(); break;
                case TokenType::LeftSquareBracket: value = derived().buildArray(); break;
                default:
                    status_ = core::JsonError::InvalidValue;
                    return JsonValue::Null();
            }

            if (has_error()) [[unlikely]] {
                return JsonValue::Null();
            }

            res_.pushArrayElement(value);

            if (current_->type_ == TokenType::Comma) {
                ++current_;
                if (current_->type_ == TokenType::RightSquareBracket) {
                    status_ = core::JsonError::TrailingComma;
                    return JsonValue::Null();
                }
                continue;
            }

            if (current_->type_ == TokenType::RightSquareBracket) {
                ++current_;
                return JsonValue::Array(res_.sealArray(start_offset));
            }

            status_ = core::JsonError::MissingComma;
            return JsonValue::Null();
        }
    }

    [[nodiscard]] JsonValue buildObject() noexcept {
        if (current_depth_ >= kMaxDepth) [[unlikely]] {
            status_ = core::JsonError::DepthLimitExceeded;
            return JsonValue::Null();
        }
        models::DepthGuard guard(current_depth_);

        ++current_;

        if (current_->type_ == TokenType::RightCurlyBracket) {
            ++current_;
            return JsonValue::Object(res_.sealObject(res_.getObjectOffset()));
        }

        const uint64_t start_offset = res_.getObjectOffset();

        while (true) {
            if (current_->type_ != TokenType::String) [[unlikely]] {
                status_ = core::JsonError::UnquotedKey;
                return JsonValue::Null();
            }

            std::string_view key_val = current_->value();
            if (current_->has_escape_) {
                key_val = derived().unescapeString(key_val);
                if (has_error()) [[unlikely]] {
                    return JsonValue::Null();
                }
            }

            JsonMember member{};
            
            if (key_val.size() <= constants::sso_max_len) {
                uint64_t sso_block = 0;
                std::memcpy(&sso_block, key_val.data(), key_val.size());
                sso_block |= static_cast<uint64_t>(constants::sso_tag | key_val.size()) << 56;
                std::memcpy(&member.key_, &sso_block, 8);
            } else {
                member.key_.ref_.length_ = static_cast<uint32_t>(key_val.size());
                member.key_.ref_.index_  = res_.commitString(key_val);
            }
            
            ++current_;

            if (current_->type_ != TokenType::Colon) [[unlikely]] {
                status_ = core::JsonError::InvalidType;
                return JsonValue::Null();
            }
            ++current_;

            JsonValue value;
            switch (current_->type_) {
                case TokenType::String:            value = derived().buildString(); break;
                case TokenType::Integer:           value = derived().buildInteger(); break;
                case TokenType::Boolean:           value = derived().buildBoolean(); break;
                case TokenType::Double:            value = derived().buildDouble(); break;
                case TokenType::Null:              value = derived().buildNull(); break;
                case TokenType::LeftCurlyBracket:  value = derived().buildObject(); break;
                case TokenType::LeftSquareBracket: value = derived().buildArray(); break;
                default:
                    status_ = core::JsonError::InvalidValue;
                    return JsonValue::Null();
            }

            if (has_error()) [[unlikely]] {
                return JsonValue::Null();
            }

            member.value_ = value;
            res_.pushObjectMember(member);

            if (current_->type_ == TokenType::Comma) [[likely]] {
                ++current_;
                if (current_->type_ == TokenType::RightCurlyBracket) [[unlikely]] {
                    status_ = core::JsonError::TrailingComma;
                    return JsonValue::Null();
                }
                continue;
            }

            if (current_->type_ == TokenType::RightCurlyBracket) [[likely]] {
                ++current_;
                return JsonValue::Object(res_.sealObject(start_offset));
            }

            status_ = core::JsonError::MissingComma;
            return JsonValue::Null();
        }
    }

    [[nodiscard]] JsonValue buildValue() noexcept {
        switch (current_->type_) {
            case TokenType::Null: return derived().buildNull();
            case TokenType::Boolean: return derived().buildBoolean();
            case TokenType::Integer: return derived().buildInteger();
            case TokenType::Double: return derived().buildDouble();
            case TokenType::String: return derived().buildString();
            case TokenType::LeftSquareBracket: return derived().buildArray();
            case TokenType::LeftCurlyBracket: return derived().buildObject();
            case TokenType::RightSquareBracket:
            case TokenType::RightCurlyBracket:
            case TokenType::Comma:
            case TokenType::Colon:
                status_ = core::JsonError::EmptyValue;
                return JsonValue::Null();
            default:
                status_ = core::JsonError::InvalidValue;
                return JsonValue::Null();
        }
    }

    void parse() noexcept {
        if (current_ == end_ || current_->type_ == TokenType::EndOfFile) {
            status_ = core::JsonError::EmptyValue;
            return;
        }

        auto root = derived().buildValue();
        if (has_error()) {
            return;
        }

        res_.setRoot(root);
        if (current_ == end_ || current_->type_ != TokenType::EndOfFile) {
            status_ = core::JsonError::InvalidValue;
        }
    }
};

} // namespace zuu::parser