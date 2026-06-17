/**
 * @file parser.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-10
 *
 * @copyright Copyright (c) 2026
 */

#include "parser/parser.hpp"
#include "utils/parse_double.hpp"
#include <charconv>

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

unsigned Parser::decodeUnicodeHex(const char* ptr) noexcept {
    unsigned value = 0;
    for (int i = 0; i < 4; ++i) {
        char c = ptr[i];
        value <<= 4;
        if (c >= '0' && c <= '9')
            value |= (c - '0');
        else if (c >= 'a' && c <= 'f')
            value |= (c - 'a' + 10);
        else if (c >= 'A' && c <= 'F')
            value |= (c - 'A' + 10);
        else {
            status_ = core::JsonError::InvalidValue;
            return 0;
        }
    }
    return value;
}

std::string_view Parser::unescapeString(std::string_view src) noexcept {
    // Alokasikan ruang buffer di dalam Memory Arena
    char* dest = res_.allocateStringBuffer(src.size());
    char* out = dest;
    const char* ptr = src.data();
    const char* end = ptr + src.size();

    while (ptr < end) {
        if (*ptr == '\\') {
            ++ptr;
            if (ptr >= end)
                break;
            switch (*ptr) {
                case '"':
                    *out++ = '"';
                    break;
                case '\\':
                    *out++ = '\\';
                    break;
                case '/':
                    *out++ = '/';
                    break;
                case 'b':
                    *out++ = '\b';
                    break;
                case 'f':
                    *out++ = '\f';
                    break;
                case 'n':
                    *out++ = '\n';
                    break;
                case 'r':
                    *out++ = '\r';
                    break;
                case 't':
                    *out++ = '\t';
                    break;
                case 'u': {
                    if (ptr + 5 > end) {
                        status_ = core::JsonError::InvalidValue;
                        return {};
                    }
                    unsigned cp = decodeUnicodeHex(ptr + 1);
                    ptr += 4;

                    // Konversi UTF-16 Surrogate Pairs ke format UTF-8
                    if (cp >= 0xD800 && cp <= 0xDBFF) {
                        if (ptr + 6 <= end && ptr[1] == '\\' && ptr[2] == 'u') {
                            unsigned cp2 = decodeUnicodeHex(ptr + 3);
                            if (cp2 >= 0xDC00 && cp2 <= 0xDFFF) {
                                cp = 0x10000 + (((cp - 0xD800) << 10) | (cp2 - 0xDC00));
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

                    // Tulis hasil UTF-8 ke tujuan memory arena
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
    return {dest, static_cast<unsigned long long>(out - dest)};
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
    long long value{};
    auto [ptr, ec] = std::from_chars(current_->begin_, current_->begin_ + current_->size_, value);

    if (ec != std::errc{} || ptr != current_->begin_ + current_->size_) {
        status_ = core::JsonError::InvalidValue;
        return Parser::JsonValue::Null();
    }
    current_++;
    return Parser::JsonValue::Integer(value);
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

    // Lazy Evaluation: Decode hanya jika terbukti ada karakter escape
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
    ++current_;

    if (current_->type_ == TokenType::RightSquareBracket) {
        ++current_;
        return JsonValue::Array(res_.sealArray(res_.getArrayOffset()));
    }

    const unsigned long long start_offset = res_.getArrayOffset();

    while (true) {
        auto value = buildValue();
        if (has_error()) {
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
    ++current_;

    if (current_->type_ == TokenType::RightCurlyBracket) {
        ++current_;
        return JsonValue::Object(res_.sealObject(res_.getObjectOffset()));
    }

    const unsigned long long start_offset = res_.getObjectOffset();

    while (true) {
        if (current_->type_ != TokenType::String) [[unlikely]] {
            status_ = core::JsonError::UnquotedKey;
            return JsonValue::Null();
        }

        // Terapkan Zero-copy fallback atau Unescaping untuk Key objek JSON
        std::string_view key_val = current_->value();
        if (current_->has_escape_) {
            key_val = unescapeString(key_val);
            if (has_error()) [[unlikely]]
                return JsonValue::Null();
        }

        const auto key_index = res_.commitString(key_val);
        ++current_;

        if (current_->type_ != TokenType::Colon) [[unlikely]] {
            status_ = core::JsonError::InvalidType;
            return JsonValue::Null();
        }
        ++current_;

        auto value = buildValue();
        if (has_error()) [[unlikely]] {
            return JsonValue::Null();
        }

        res_.pushObjectMember(JsonMember{.key_index_ = key_index, .value_ = value});

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
        case TokenType::Null:
            return buildNull();
        case TokenType::Boolean:
            return buildBoolean();
        case TokenType::Integer:
            return buildInteger();
        case TokenType::Double:
            return buildDouble();
        case TokenType::String:
            return buildString();
        case TokenType::LeftSquareBracket:
            return buildArray();
        case TokenType::LeftCurlyBracket:
            return buildObject();
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