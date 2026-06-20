/**
 * @file parser.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-10
 *
 * @copyright Copyright (c) 2026
 */

#include "constants/general.hpp"
#include <cstdint>
#include <cstring>
#include "models/depth_guard.hpp"
#include "parser/parser.hpp"
#include "utils/parser.hpp"
#include "utils/swar.hpp"

namespace zuu::parser {

Parser::Parser(Raw tokens, Hint hint) noexcept
    : current_(tokens.data())
    , end_(tokens.data() + tokens.size())
    , res_(hint) {
    parse();
}

Parser::Expected Parser::result() && noexcept {
    if (has_error()) {
        return std::unexpected{status_};
    }
    return std::move(res_);
}

bool Parser::has_error() const noexcept {
    return status_ != Error::None;
}

Parser::Expected Parser::Parse(Raw tokens, Hint hint) noexcept {
    return Parser(tokens, hint).result();
}

uint32_t Parser::decodeUnicodeHex(const char* ptr) noexcept {
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

std::string_view Parser::unescapeString(std::string_view src) noexcept {
    char* dest = res_.allocateStringBuffer(src.size());
    char* out = dest;
    const char* ptr = src.data();
    const char* end = ptr + src.size();

    while (ptr < end) {
        while (ptr + sizeof(uint64_t) <= end) {
            uint64_t block{};
            std::memcpy(
                &block, 
                ptr, 
                sizeof(uint64_t)
            );

            uint64_t mask = utils::find_zero_byte_mask(block ^ constants::swar8_escape);
            if (mask != 0) {
                uint32_t offset = std::countr_zero(mask) >> 3;
                std::memcpy(out, ptr, offset);

                out += offset;
                ptr += offset;
                
                break;
            }

            std::memcpy(
                out, 
                ptr, 
                sizeof(uint64_t)
            );

            out += sizeof(uint64_t);
            ptr += sizeof(uint64_t);
        }

        if (ptr >= end) {
            break; 
        }

        if (*ptr == '\\') {
            ++ptr;
            if (ptr >= end) {
                break;
            }
            
            switch (*ptr) {
                case '"':  *out++ = '"';  break;
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
                        return {};
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
                                return {};
                            }
                        } else {
                            status_ = core::JsonError::InvalidValue;
                            return {};
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
                        return {};
                    }
                    break;
                }
                default:
                    status_ = core::JsonError::InvalidValue;
                    return {};
            }
        } else {
            *out++ = *ptr;
        }
        ++ptr;
    }
    return {dest, static_cast<uint64_t>(out - dest)};
}

Parser::JsonValue Parser::buildNull() noexcept {
    current_++;
    return Parser::JsonValue::Null();
}

Parser::JsonValue Parser::buildBoolean() noexcept {
    const auto value = current_->begin_[0] == 't';
    current_++;
    return Parser::JsonValue::Boolean(value);
}

Parser::JsonValue Parser::buildInteger() noexcept {
    if(
        auto res = utils::parse<long long>(
            current_->begin_, 
            current_->begin_ + current_->size_
        );
        res
    ) {
        current_++;
        return Parser::JsonValue::Integer(*res);
    } else {
        status_ = core::JsonError::InvalidValue;
        return Parser::JsonValue::Null();
    }
}

Parser::JsonValue Parser::buildDouble() noexcept {
    if(
        auto res = utils::parse<double>(
            current_->begin_, 
            current_->begin_ + current_->size_
        );
        res
    ) {
        current_++;
        return Parser::JsonValue::Double(*res);
    } else {
        status_ = core::JsonError::InvalidValue;
        return Parser::JsonValue::Null();
    }
}

Parser::JsonValue Parser::buildString() noexcept {
    std::string_view val = current_->value();

    if (current_->has_escape_) {
        val = unescapeString(val);
        if (has_error()) {
            return JsonValue::Null();
        }
    }

    const auto index = res_.commitString(val);
    ++current_;
    return Parser::JsonValue::String(index);
}

Parser::JsonValue Parser::buildArray() noexcept {
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
        Parser::JsonValue value;
        switch (current_->type_) {
            case TokenType::String:            value = buildString(); break;
            case TokenType::Integer:           value = buildInteger(); break;
            case TokenType::Boolean:           value = buildBoolean(); break;
            case TokenType::Double:            value = buildDouble(); break;
            case TokenType::Null:              value = buildNull(); break;
            case TokenType::LeftCurlyBracket:  value = buildObject(); break;
            case TokenType::LeftSquareBracket: value = buildArray(); break;
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
                return models::JsonValue::Null();
            }
            continue;
        }

        if (current_->type_ == TokenType::RightSquareBracket) {
            ++current_;
            return models::JsonValue::Array(res_.sealArray(start_offset));
        }

        status_ = core::JsonError::MissingComma;
        return models::JsonValue::Null();
    }
}

Parser::JsonValue Parser::buildObject() noexcept {
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
            key_val = unescapeString(key_val);
            if (has_error()) [[unlikely]] {
                return JsonValue::Null();
            }
        }

        JsonMember member{};
        
        if (key_val.size() <= constants::sso_max_len) [[likely]] {
            std::memcpy(member.key_.sso_.chars_, key_val.data(), key_val.size());
            member.key_.sso_.length_tag_ = constants::sso_tag | static_cast<unsigned char>(key_val.size());
        } else {
            member.key_.index_ = res_.commitString(key_val);
        }
        ++current_;

        if (current_->type_ != TokenType::Colon) [[unlikely]] {
            status_ = core::JsonError::InvalidType;
            return JsonValue::Null();
        }
        ++current_;

        Parser::JsonValue value;
        switch (current_->type_) {
            case TokenType::String: value = buildString(); break;
            case TokenType::Integer: value = buildInteger(); break;
            case TokenType::Boolean: value = buildBoolean(); break;
            case TokenType::Double: value = buildDouble(); break;
            case TokenType::Null: value = buildNull(); break;
            case TokenType::LeftCurlyBracket: value = buildObject(); break;
            case TokenType::LeftSquareBracket: value = buildArray(); break;
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

Parser::JsonValue Parser::buildValue() noexcept {
    switch (current_->type_) {
        case TokenType::Null: return buildNull();
        case TokenType::Boolean: return buildBoolean();
        case TokenType::Integer: return buildInteger();
        case TokenType::Double: return buildDouble();
        case TokenType::String: return buildString();
        case TokenType::LeftSquareBracket: return buildArray();
        case TokenType::LeftCurlyBracket: return buildObject();
        case TokenType::RightSquareBracket:
        case TokenType::RightCurlyBracket:
        case TokenType::Comma:
        case TokenType::Colon:
            status_ = core::JsonError::EmptyValue;
            return Parser::JsonValue::Null();
        default:
            status_ = core::JsonError::InvalidValue;
            return Parser::JsonValue::Null();
    }
}

void Parser::parse() noexcept {
    if (current_ == end_ || current_->type_ == TokenType::EndOfFile) {
        status_ = core::JsonError::EmptyValue;
        return;
    }

    auto root = buildValue();
    if (has_error()) {
        return;
    }

    res_.setRoot(root);
    if (current_ == end_ || current_->type_ != TokenType::EndOfFile) {
        status_ = core::JsonError::InvalidValue;
    }
}

} // namespace zuu::parser