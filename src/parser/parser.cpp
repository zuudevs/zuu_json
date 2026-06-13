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
#include <vector>

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

std::string_view Parser::parseStringToken(const Token& token) noexcept {
    // Alokasi buffer dari bump allocator — hasilnya stabil selama Storage hidup
    char* buf = res_.allocateStringBuffer(token.size_);
    size_t write_pos = 0;

    const char* ptr = token.ptr_;
    const size_t len = token.size_;

    for (size_t i = 0; i < len; ++i) {
        if (ptr[i] == '\\') {
            if (i + 1 >= len) {
                status_ = core::JsonError::UnescapedCharacter;
                return {};
            }
            i++; // Lewati backslash

            switch (ptr[i]) {
                case '"':
                    buf[write_pos++] = '"';
                    break;
                case '\\':
                    buf[write_pos++] = '\\';
                    break;
                case '/':
                    buf[write_pos++] = '/';
                    break;
                case 'b':
                    buf[write_pos++] = '\b';
                    break;
                case 'f':
                    buf[write_pos++] = '\f';
                    break;
                case 'n':
                    buf[write_pos++] = '\n';
                    break;
                case 'r':
                    buf[write_pos++] = '\r';
                    break;
                case 't':
                    buf[write_pos++] = '\t';
                    break;
                case 'u': {
                    if (i + 4 >= len) {
                        status_ = core::JsonError::InvalidUnicode;
                        return {};
                    }

                    // Baca 4 digit hex (\uXXXX)
                    uint32_t cp = 0;
                    for (int j = 1; j <= 4; ++j) {
                        int hex = utils::hex_to_int(ptr[i + j]);
                        if (hex < 0) {
                            status_ = core::JsonError::InvalidUnicode;
                            return {};
                        }
                        cp = (cp << 4) | hex;
                    }
                    i += 4;

                    // Validasi Surrogate Pair (Kombinasi 2 codepoint untuk karakter > 16-bit
                    // seperti Emoji)
                    if (cp >= 0xD800 && cp <= 0xDBFF) { // High Surrogate
                        if (i + 6 >= len || ptr[i + 1] != '\\' || ptr[i + 2] != 'u') {
                            status_ = core::JsonError::InvalidSurrogate;
                            return {};
                        }

                        uint32_t cp2 = 0;
                        for (int j = 3; j <= 6; ++j) {
                            int hex = utils::hex_to_int(ptr[i + j]);
                            if (hex < 0) {
                                status_ = core::JsonError::InvalidSurrogate;
                                return {};
                            }
                            cp2 = (cp2 << 4) | hex;
                        }

                        if (cp2 < 0xDC00 || cp2 > 0xDFFF) { // Low Surrogate
                            status_ = core::JsonError::InvalidSurrogate;
                            return {};
                        }

                        // Gabungkan menjadi codepoint aslinya
                        cp = 0x10000 + (((cp - 0xD800) << 10) | (cp2 - 0xDC00));
                        i += 6;
                    } else if (cp >= 0xDC00 && cp <= 0xDFFF) {
                        // Low Surrogate yg berdiri sendirian itu terlarang
                        status_ = core::JsonError::InvalidSurrogate;
                        return {};
                    }

                    // Tulis Codepoint menjadi bytes UTF-8 ke dalam buffer
                    // UTF-8 encoding: max 4 bytes per codepoint
                    if (cp <= 0x7F) {
                        buf[write_pos++] = static_cast<char>(cp);
                    } else if (cp <= 0x7FF) {
                        buf[write_pos++] = static_cast<char>(0xC0 | ((cp >> 6) & 0x1F));
                        buf[write_pos++] = static_cast<char>(0x80 | (cp & 0x3F));
                    } else if (cp <= 0xFFFF) {
                        buf[write_pos++] = static_cast<char>(0xE0 | ((cp >> 12) & 0x0F));
                        buf[write_pos++] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                        buf[write_pos++] = static_cast<char>(0x80 | (cp & 0x3F));
                    } else if (cp <= 0x10FFFF) {
                        buf[write_pos++] = static_cast<char>(0xF0 | ((cp >> 18) & 0x07));
                        buf[write_pos++] = static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
                        buf[write_pos++] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                        buf[write_pos++] = static_cast<char>(0x80 | (cp & 0x3F));
                    }
                    break;
                }
                default:
                    status_ = core::JsonError::UnescapedCharacter;
                    return {};
            }
        } else {
            // Karakter ASCII normal atau UTF-8 mentah (passthrough)
            buf[write_pos++] = ptr[i];
        }
    }
    return {buf, write_pos};
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
    // Zero-copy path: string tanpa escape langsung pakai view ke raw token
    if (!raw_[idx_].has_escape_) {
        const auto index = res_.commitString(raw_[idx_].value());
        idx_++;
        return Parser::JsonValue::String(index);
    }

    // Escaped path: unescape ke bump allocator buffer
    auto parsed_sv = parseStringToken(raw_[idx_]);
    if (has_error())
        return Parser::JsonValue::Null();

    const auto index = res_.commitString(parsed_sv);
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

    // Kumpulkan semua child value terlebih dahulu, agar nested container
    // selesai menyimpan elemen mereka ke buffer sebelum parent memulai push.
    // Ini mencegah interleaving di flat bump allocator.
    thread_local std::vector<JsonValue> temp_values;
    const size_t saved_size = temp_values.size();

    while (true) {
        auto value = buildValue();
        if (has_error()) {
            temp_values.resize(saved_size);
            return JsonValue::Null();
        }

        temp_values.push_back(value);

        if (raw_[idx_].type_ == TokenType::Comma) {
            ++idx_;
            if (raw_[idx_].type_ == TokenType::RightSquareBracket) {
                status_ = core::JsonError::TrailingComma;
                temp_values.resize(saved_size);
                return models::JsonValue::Null();
            }
            continue;
        }

        if (raw_[idx_].type_ == TokenType::RightSquareBracket) {
            ++idx_;
            // Push semua elemen secara kontinu ke bump allocator
            const size_t start_offset = res_.getArrayOffset();
            for (size_t i = saved_size; i < temp_values.size(); ++i) {
                res_.pushArrayElement(temp_values[i]);
            }
            temp_values.resize(saved_size);
            return models::JsonValue::Array(res_.sealArray(start_offset));
        }

        status_ = core::JsonError::MissingComma;
        temp_values.resize(saved_size);
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

    // Sama seperti buildArray — kumpulkan dulu semua member,
    // baru push sekaligus agar tidak interleaving dengan nested object.
    thread_local std::vector<JsonMember> temp_members;
    const size_t saved_size = temp_members.size();

    while (true) {
        if (raw_[idx_].type_ != TokenType::String) {
            status_ = core::JsonError::UnquotedKey;
            temp_members.resize(saved_size);
            return JsonValue::Null();
        }

        // Terapkan Zero-copy fallback atau Unescaping untuk Key objek JSON
        std::string_view key_val = raw_[idx_].value();
        if (raw_[idx_].has_escape_) {
            key_val = parseStringToken(raw_[idx_]);
            if (has_error()) {
                temp_members.resize(saved_size);
                return JsonValue::Null();
            }
        }

        const auto key_index = res_.commitString(key_val);
        ++idx_;

        if (raw_[idx_].type_ != TokenType::Colon) {
            status_ = core::JsonError::InvalidType;
            temp_members.resize(saved_size);
            return JsonValue::Null();
        }
        ++idx_;

        auto value = buildValue();
        if (has_error()) {
            temp_members.resize(saved_size);
            return JsonValue::Null();
        }

        temp_members.push_back(JsonMember{.key_index_ = key_index, .value_ = value});

        if (raw_[idx_].type_ == TokenType::Comma) {
            ++idx_;
            if (raw_[idx_].type_ == TokenType::RightCurlyBracket) {
                status_ = core::JsonError::TrailingComma;
                temp_members.resize(saved_size);
                return JsonValue::Null();
            }
            continue;
        }

        if (raw_[idx_].type_ == TokenType::RightCurlyBracket) {
            ++idx_;
            // Push semua member secara kontinu ke bump allocator
            const size_t start_offset = res_.getObjectOffset();
            for (size_t i = saved_size; i < temp_members.size(); ++i) {
                res_.pushObjectMember(temp_members[i]);
            }
            temp_members.resize(saved_size);
            return JsonValue::Object(res_.sealObject(start_offset));
        }

        status_ = core::JsonError::MissingComma;
        temp_members.resize(saved_size);
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