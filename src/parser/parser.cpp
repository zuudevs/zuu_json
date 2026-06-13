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

Parser::Parser(Resource resource) noexcept
    : raw_(resource.first), res_(resource.second) {
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

Parser::Expected Parser::Parse(Resource resource) noexcept {
    return Parser(resource).result();
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
            i++; // Lewati backslash

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

                    // Baca 4 digit hex (\uXXXX)
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

                    // Validasi Surrogate Pair (Kombinasi 2 codepoint untuk karakter > 16-bit
                    // seperti Emoji)
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
            // Karakter ASCII normal atau UTF-8 mentah (passthrough)
            result.push_back(ptr[i]);
        }
    }
    return result;
}

Parser::JsonValue Parser::buildNull() noexcept {
    idx_++;
    return Parser::JsonValue::Null();
}

Parser::JsonValue Parser::buildBoolean() noexcept {
    const auto value = raw_[idx_].ptr_[0] == 't';
    idx_++;
    return Parser::JsonValue::Boolean(value);
}

Parser::JsonValue Parser::buildInteger() noexcept {
    long long value{};
    auto [ptr, ec] = std::from_chars(raw_[idx_].ptr_, raw_[idx_].ptr_ + raw_[idx_].size_, value);

    if (ec != std::errc{} || ptr != raw_[idx_].ptr_ + raw_[idx_].size_) {
        status_ = core::JsonError::InvalidValue;
        return Parser::JsonValue::Null();
    }
    idx_++;
    return Parser::JsonValue::Integer(value);
}

Parser::JsonValue Parser::buildDouble() noexcept {
    long double value{};
    auto [ptr, ec] = std::from_chars(raw_[idx_].ptr_, raw_[idx_].ptr_ + raw_[idx_].size_, value);

    if (ec != std::errc{} || ptr != raw_[idx_].ptr_ + raw_[idx_].size_) {
        status_ = core::JsonError::InvalidValue;
        return Parser::JsonValue::Null();
    }
    idx_++;
    return Parser::JsonValue::Double(value);
}

Parser::JsonValue Parser::buildString() noexcept {
    // Jalankan mesin unescape untuk nilai String
    auto parsed_str = parseStringToken(raw_[idx_]);
    if (has_error())
        return Parser::JsonValue::Null();

    const auto index = res_.commitString(parsed_str);
    idx_++;
    return Parser::JsonValue::String(index);
}

Parser::JsonValue Parser::buildArray() noexcept {
    idx_++;

    if (idx_ >= raw_.size()) {
        status_ = core::JsonError::InvalidValue;
        return Parser::JsonValue::Null();
    }

    if (raw_[idx_].type_ == TokenType::RightSquareBracket) {
        idx_++;
        return Parser::JsonValue::Array(res_.sealArray(res_.getArrayOffset()));
    }

    const size_t start_offset = res_.getArrayOffset();

    while (true) {
        auto value = buildValue();
        if (has_error()) {
            return JsonValue::Null();
        }

        res_.pushArrayElement(value);

        if (raw_[idx_].type_ == TokenType::Comma) {
            ++idx_;
            if (raw_[idx_].type_ == TokenType::RightSquareBracket) {
                status_ = core::JsonError::TrailingComma;
                return models::JsonValue::Null();
            }
            continue;
        }

        if (raw_[idx_].type_ == TokenType::RightSquareBracket) {
            ++idx_;
            return models::JsonValue::Array(res_.sealArray(start_offset));
        }

        status_ = core::JsonError::MissingComma;
        return models::JsonValue::Null();
    }
}

Parser::JsonValue Parser::buildObject() noexcept {
    idx_++;

    if (idx_ >= raw_.size()) {
        status_ = core::JsonError::InvalidValue;
        return Parser::JsonValue::Null();
    }

    if (raw_[idx_].type_ == TokenType::RightCurlyBracket) {
        idx_++;
        return Parser::JsonValue::Object(res_.sealObject(res_.getObjectOffset()));
    }

    const size_t start_offset = res_.getObjectOffset();

    while (true) {
        if (raw_[idx_].type_ != TokenType::String) {
            status_ = core::JsonError::UnquotedKey;
            return JsonValue::Null();
        }

        // Terapkan Zero-copy fallback atau Unescaping untuk Key objek JSON
        std::string_view key_val = raw_[idx_].value();
		std::string key_buf;
        if (raw_[idx_].has_escape_) {
            key_buf = parseStringToken(raw_[idx_]);
            if (has_error())
                return JsonValue::Null();
			key_val = key_buf;
        }

        const auto key_index = res_.commitString(key_val);
        ++idx_;

        if (raw_[idx_].type_ != TokenType::Colon) {
            status_ = core::JsonError::InvalidType;
            return JsonValue::Null();
        }
        ++idx_;

        auto value = buildValue();
        if (has_error()) {
            return JsonValue::Null();
        }

        res_.pushObjectMember(JsonMember{.key_index_ = key_index, .value_ = value});

        if (raw_[idx_].type_ == TokenType::Comma) {
            ++idx_;
            if (raw_[idx_].type_ == TokenType::RightCurlyBracket) {
                status_ = core::JsonError::TrailingComma;
                return JsonValue::Null();
            }
            continue;
        }

        if (raw_[idx_].type_ == TokenType::RightCurlyBracket) {
            ++idx_;
            return JsonValue::Object(res_.sealObject(start_offset));
        }

        status_ = core::JsonError::MissingComma;
        return JsonValue::Null();
    }
}

Parser::JsonValue Parser::buildValue() noexcept {
    if (idx_ >= raw_.size()) {
        status_ = core::JsonError::EmptyValue;
        return Parser::JsonValue::Null();
    }

    switch (raw_[idx_].type_) {
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
    if (raw_.empty() || raw_[idx_].type_ == TokenType::EndOfFile) {
        status_ = core::JsonError::EmptyValue;
        return;
    }

    auto root = buildValue();
    if (has_error()) {
        return;
    }

    res_.setRoot(root);
    if (raw_.empty() || raw_[idx_].type_ != TokenType::EndOfFile) {
        status_ = core::JsonError::InvalidValue;
    }
}

} // namespace zuu::parser