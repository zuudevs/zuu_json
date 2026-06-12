/**
 * @file parser.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-10
 *
 * @copyright Copyright (c) 2026
 */

#include "parser/parser.hpp"
#include "utils/strings.hpp"
#include <charconv>

namespace zuu::parser {

Parser::Parser(Parser::Raw tokens) noexcept
    : current_(tokens.data())
	, end_(tokens.data() + tokens.size()) {
    res_ = Storage(getStringCount(), getArrayCount(), getObjectCount());
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

Parser::Expected Parser::Parse(Parser::Raw tokens) noexcept {
    return Parser(tokens).result();
}

size_t Parser::getStringCount() const noexcept {
    size_t n{0};
	for (auto it = current_; it != end_; ++it) {
		if (it->type_ == TokenType::String) {
            n++;
        }
	}
    return n;
}

size_t Parser::getArrayCount() const noexcept {
    size_t n{0};
    for (auto it = current_; it != end_; ++it) {
		if (it->type_ == TokenType::LeftSquareBracket) {
            n++;
        }
    }
    return n;
}

size_t Parser::getObjectCount() const noexcept {
    size_t n{0};
    for (auto it = current_; it != end_; ++it) {
		if (it->type_ == TokenType::LeftCurlyBracket) {
            n++;
        }
    }
    return n;
}

std::string Parser::parseStringToken(const Token& token) noexcept {
    std::string result;
    result.reserve(token.size_);

    const char* ptr = token.ptr_;
    const size_t len = token.size_;

    for (size_t i = 0; i < len; ++i) {
        if (ptr[i] == '\\') {
            if (i + 1 >= len) {
                status_ = core::JsonError::UnescapedCharacter;
                return "";
            }
            i++;

            switch (ptr[i]) {
                case '"':
                    result.push_back('"');
                    break;
                case '\\':
                    result.push_back('\\');
                    break;
                case '/':
                    result.push_back('/');
                    break;
                case 'b':
                    result.push_back('\b');
                    break;
                case 'f':
                    result.push_back('\f');
                    break;
                case 'n':
                    result.push_back('\n');
                    break;
                case 'r':
                    result.push_back('\r');
                    break;
                case 't':
                    result.push_back('\t');
                    break;
                case 'u': {
                    if (i + 4 >= len) {
                        status_ = core::JsonError::InvalidUnicode;
                        return "";
                    }

                    uint32_t cp = 0;
                    for (int j = 1; j <= 4; ++j) {
                        int hex = utils::hex_to_int(ptr[i + j]);
                        if (hex < 0) {
                            status_ = core::JsonError::InvalidUnicode;
                            return "";
                        }
                        cp = (cp << 4) | hex;
                    }
                    i += 4;

                    if (cp >= 0xD800 && cp <= 0xDBFF) { // High Surrogate
                        if (i + 6 >= len || ptr[i + 1] != '\\' || ptr[i + 2] != 'u') {
                            status_ = core::JsonError::InvalidSurrogate;
                            return "";
                        }

                        uint32_t cp2 = 0;
                        for (int j = 3; j <= 6; ++j) {
                            int hex = utils::hex_to_int(ptr[i + j]);
                            if (hex < 0) {
                                status_ = core::JsonError::InvalidSurrogate;
                                return "";
                            }
                            cp2 = (cp2 << 4) | hex;
                        }

                        if (cp2 < 0xDC00 || cp2 > 0xDFFF) { // Low Surrogate
                            status_ = core::JsonError::InvalidSurrogate;
                            return "";
                        }

                        // Gabungkan menjadi codepoint aslinya
                        cp = 0x10000 + (((cp - 0xD800) << 10) | (cp2 - 0xDC00));
                        i += 6;
                    } else if (cp >= 0xDC00 && cp <= 0xDFFF) {
                        // Low Surrogate yg berdiri sendirian itu terlarang
                        status_ = core::JsonError::InvalidSurrogate;
                        return "";
                    }

                    // Tulis Codepoint menjadi bytes UTF-8 ke dalam string
                    utils::encode_utf8(cp, result);
                    break;
                }
                default:
                    status_ = core::JsonError::UnescapedCharacter;
                    return "";
            }
        } else {
            result.push_back(ptr[i]);
        }
    }
    return result;
}

Parser::JsonValue Parser::buildNull() noexcept {
    current_++;
    return Parser::JsonValue::Null();
}

Parser::JsonValue Parser::buildBoolean() noexcept {
    const auto value = current_->value()[0] == 't';
    current_++;
    return Parser::JsonValue::Boolean(value);
}

Parser::JsonValue Parser::buildInteger() noexcept {
    long long value{};
    auto [ptr, ec] = std::from_chars(current_->ptr_, current_->ptr_ + current_->size_, value);

    if (ec != std::errc{} || ptr != current_->ptr_ + current_->size_) {
        status_ = core::JsonError::InvalidValue;
        return Parser::JsonValue::Null();
    }
    current_++;
    return Parser::JsonValue::Integer(value);
}

Parser::JsonValue Parser::buildDouble() noexcept {
    long double value{};
    auto [ptr, ec] = std::from_chars(current_->ptr_, current_->ptr_ + current_->size_, value);

    if (ec != std::errc{} || ptr != current_->ptr_ + current_->size_) {
        status_ = core::JsonError::InvalidValue;
        return Parser::JsonValue::Null();
    }
    current_++;
    return Parser::JsonValue::Double(value);
}

Parser::JsonValue Parser::buildString() noexcept {
    // Jalankan mesin unescape untuk nilai String
    auto parsed_str = parseStringToken(*current_);
    if (has_error())
        return Parser::JsonValue::Null();

    const auto index = res_.addString(parsed_str);
    current_++;
    return Parser::JsonValue::String(index);
}

Parser::JsonValue Parser::buildArray() noexcept {
    const auto array_index = res_.addArray();
    current_++;

    if (current_ >= end_) {
        status_ = core::JsonError::InvalidValue;
        return Parser::JsonValue::Null();
    }

    if (current_->type_ == TokenType::RightSquareBracket) {
        current_++;
        return Parser::JsonValue::Array(array_index);
    }

    while (current_ < end_) {
        if (current_->type_ == TokenType::RightSquareBracket || current_->type_ == TokenType::Comma) {
            status_ = core::JsonError::EmptyValue;
            return Parser::JsonValue::Null();
        }

        auto value = buildValue();
        if (has_error()) {
            return Parser::JsonValue::Null();
        }

        res_.array(array_index).push_back(value);

        if (current_ >= end_) {
            status_ = core::JsonError::InvalidValue;
            return Parser::JsonValue::Null();
        }

        if (current_->type_ == TokenType::Comma) {
            current_++;
            if (current_ < end_ && current_->type_ == TokenType::RightSquareBracket) {
                status_ = core::JsonError::TrailingComma;
                return Parser::JsonValue::Null();
            }
            continue;
        }

        if (current_->type_ == TokenType::RightSquareBracket) {
            current_++;
            return Parser::JsonValue::Array(array_index);
        }

        status_ = core::JsonError::MissingComma;
        return Parser::JsonValue::Null();
    }

    status_ = core::JsonError::InvalidValue;
    return Parser::JsonValue::Null();
}

Parser::JsonValue Parser::buildObject() noexcept {
    const auto object_index = res_.addObject();
    current_++;

    if (current_ >= end_) {
        status_ = core::JsonError::InvalidValue;
        return Parser::JsonValue::Null();
    }

    if (current_->type_ == TokenType::RightCurlyBracket) {
        current_++;
        return Parser::JsonValue::Object(object_index);
    }

    while (current_ < end_) {
        if (current_->type_ != TokenType::String) {
            status_ = core::JsonError::UnquotedKey;
            return Parser::JsonValue::Null();
        }

        // Jalankan mesin unescape untuk Kunci (Key) Object
        auto parsed_key = parseStringToken(*current_);
        if (has_error())
            return Parser::JsonValue::Null();

        const auto key_index = res_.addString(parsed_key);
        current_++;

        if (current_ >= end_ || current_->type_ != TokenType::Colon) {
            status_ = core::JsonError::InvalidType;
            return Parser::JsonValue::Null();
        }
        current_++;

        if (current_ >= end_) {
            status_ = core::JsonError::EmptyValue;
            return Parser::JsonValue::Null();
        }

        if (current_->type_ == TokenType::RightCurlyBracket || current_->type_ == TokenType::Comma) {
            status_ = core::JsonError::EmptyValue;
            return Parser::JsonValue::Null();
        }

        auto value = buildValue();
        if (has_error()) {
            return Parser::JsonValue::Null();
        }

        res_.object(object_index)
            .push_back(Parser::JsonMember{.key_index_ = key_index, .value_ = value});

        if (current_ >= end_) {
            status_ = core::JsonError::InvalidValue;
            return Parser::JsonValue::Null();
        }

        if (current_->type_ == TokenType::Comma) {
            current_++;
            if (current_ < end_ && current_->type_ == TokenType::RightCurlyBracket) {
                status_ = core::JsonError::TrailingComma;
                return Parser::JsonValue::Null();
            }
            continue;
        }

        if (current_->type_ == TokenType::RightCurlyBracket) {
            current_++;
            return Parser::JsonValue::Object(object_index);
        }

        status_ = core::JsonError::MissingComma;
        return Parser::JsonValue::Null();
    }

    status_ = core::JsonError::InvalidValue;
    return Parser::JsonValue::Null();
}

Parser::JsonValue Parser::buildValue() noexcept {
    if (current_ >= end_) {
        status_ = core::JsonError::EmptyValue;
        return Parser::JsonValue::Null();
    }

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
    if (current_ == end_ || current_->type_ != Token::Type::EndOfFile) {
        status_ = core::JsonError::InvalidValue;
    }
}

} // namespace zuu::parser