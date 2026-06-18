/**
 * @file tokenizer.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-06
 *
 * @copyright Copyright (c) 2026
 */

#include <bit>
#include "constants/general.hpp"
#include <cstring>
#include "tokenizer/tokenizer.hpp"

namespace {

[[nodiscard]] inline constexpr bool has_zero_byte(unsigned long long v) noexcept {
    return (v - 0x0101010101010101ULL) & ~v & 0x8080808080808080ULL;
}

[[nodiscard]] inline constexpr bool has_quote_or_escape(unsigned long long v) noexcept {
    const unsigned long long quote_mask = v ^ 0x2222222222222222ULL;
    const unsigned long long escape_mask = v ^ 0x5C5C5C5C5C5C5C5CULL;
    return has_zero_byte(quote_mask) | has_zero_byte(escape_mask);
}

// --- COMPILE-TIME SWAR CONSTANTS ---
// Struktur 4 byte persis untuk memanipulasi std::bit_cast
struct Bytes4 { char c[4]; };

// consteval: Memaksa fungsi ini dieksekusi SAAT KOMPILASI, 0% beban saat runtime!
[[nodiscard]] consteval unsigned make_word(const char* str) noexcept {
	return std::bit_cast<unsigned>(Bytes4{str[0], str[1], str[2], str[3]});
}

// Nilai-nilai ini akan menjadi angka konstan 32-bit yang menyesuaikan 
// dengan Endianness arsitektur CPU Rara (Little-Endian / Big-Endian)
constexpr unsigned null_word = make_word("null");
constexpr unsigned true_word = make_word("true");
constexpr unsigned fals_word = make_word("fals");

} // namespace

namespace zuu::tokenizer {

Tokenizer::Tokenizer(std::span<const char> json_content) noexcept
    : current_(json_content.data())
    , end_(json_content.data() + json_content.size()) {

    res_.reserve((json_content.size() >> 1) + constants::word);
    tokenize();
}

Tokenizer::Expected Tokenizer::result() && noexcept {
    if (is_error()) {
        return std::unexpected{status_};
    }
    return std::pair{std::move(res_), hint_};
}

Tokenizer::Expected Tokenizer::Tokenize(Raw json_content) noexcept {
    return Tokenizer(json_content).result();
}

bool Tokenizer::is_error() const noexcept {
    return status_ != Error::None;
}

void Tokenizer::readString() noexcept {
    const char* begin = ++current_;
    const char* ptr = begin;
    const char* end = end_;
    bool has_escape = false;

    // Fast-path SWAR: Pindai 8 bytes sekaligus
    while (ptr + constants::byte <= end) {
        unsigned long long block{};
        std::memcpy(
			&block, 
			ptr, 
			constants::byte
		);

        if (has_quote_or_escape(block)) {
            break; // Jika ada '"' atau '\', keluar dan evaluasi secara presisi di slow-path
        }
        ptr += constants::byte;
    }

    // Slow-path skalar untuk resolusi akhir dan tracking escape character
    while (ptr < end) {
        char c = *ptr;
        if (c == '"') {
            res_.emplace_back(
                Token::Type::String, 
				std::string_view(begin, ptr - begin), 
				has_escape
			);

            // Catat kebutuhan buffer jika string memiliki escape
            if (has_escape) {
                hint_.string_escape_bytes_ += (ptr - begin);
            }

            current_ = ptr + 1;
            return;
        }
        if (c == '\\') {
            has_escape = true;
            ptr += 2; // Lewati karakter escape
            if (ptr > end) {
                status_ = core::JsonError::InvalidValue;
                return;
            }
            continue;
        }
        ++ptr;
    }

    status_ = core::JsonError::InvalidValue;
}

void Tokenizer::readNumeric() noexcept {
    auto begin = current_;
    auto type = Token::Type::Integer;

    if (current_ < end_ && *current_ == '-') {
        current_++;
    }

    if (current_ < end_ && *current_ == '0') {
        current_++;
        if (current_ < end_ && (static_cast<unsigned char>(*current_ - '0') < constants::digit)) {
            status_ = Error::LeadingZero;
            return;
        }
    } else if (current_ < end_ &&
               (static_cast<unsigned char>(*current_ - '0') < constants::digit)) {
        while (current_ < end_ &&
               (static_cast<unsigned char>(*current_ - '0') < constants::digit)) {
            current_++;
        }
    } else {
        status_ = Error::InvalidValue;
        return;
    }

    if (current_ < end_ && *current_ == '.') {
        type = Token::Type::Double;
        current_++;

        if (current_ >= end_ || (static_cast<unsigned char>(*current_ - '0') >= constants::digit)) {
            status_ = Error::InvalidValue;
            return;
        }

        while (current_ < end_ &&
               (static_cast<unsigned char>(*current_ - '0') < constants::digit)) {
            current_++;
        }
    }

    if (current_ < end_ && (*current_ == 'e' || *current_ == 'E')) {
        type = Token::Type::Double;
        current_++;

        if (current_ < end_ && (*current_ == '+' || *current_ == '-')) {
            current_++;
        }

        if (current_ >= end_ || (static_cast<unsigned char>(*current_ - '0') >= constants::digit)) {
            status_ = Error::InvalidValue;
            return;
        }

        while (current_ < end_ &&
               (static_cast<unsigned char>(*current_ - '0') < constants::digit)) {
            current_++;
        }
    }

    if (!is_error()) {
        res_.emplace_back(type, std::string_view(begin, current_ - begin));
    }
}

void Tokenizer::readAlphabet() noexcept {
    const auto rem = end_ - current_;
    
    // Tarik 4 byte sekaligus dan bandingkan 32-bit angkanya!
    if (rem >= 4) [[likely]] {
        unsigned val;
        std::memcpy(&val, current_, 4); // Di-optimasi compiler jadi 1x MOV

        switch (val) {
            case null_word:
                res_.emplace_back(Token::Type::Null, std::string_view(current_, 4));
                current_ += 4;
                return;
                
            case true_word:
                res_.emplace_back(Token::Type::Boolean, std::string_view(current_, 4));
                current_ += 4;
                return;
                
            case fals_word:
                // "false" panjangnya 5, jadi kita cek byte terakhirnya
                if (rem >= 5 && current_[4] == 'e') {
                    res_.emplace_back(Token::Type::Boolean, std::string_view(current_, 5));
                    current_ += 5;
                    return;
                }
                break;
			default:
				break;
        }
    }
	status_ = Error::InvalidValue;
}

void Tokenizer::tokenize() noexcept {
    while (current_ < end_) {
        auto actionType = Lookup{}[*current_];
        switch (actionType) {
            case Lookup::Type::WhiteSpace: {
                current_++;
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
            case Lookup::Type::Colon: {
                res_.emplace_back(Token::Type::Colon);
                current_++;
                continue;
            }
            case Lookup::Type::Comma: {
                res_.emplace_back(Token::Type::Comma);
                hint_.comma_count_++;
                current_++;
                continue;
            }
            case Lookup::Type::DoubleQuote: {
                readString();
                if (is_error())
                    return;
                hint_.string_count_++;
                continue;
            }
            case Lookup::Type::Numeric: {
                readNumeric();
                if (is_error())
                    return;
                continue;
            }
            case Lookup::Type::Alphabet: {
                readAlphabet();
                if (is_error())
                    return;
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

} // namespace zuu::tokenizer