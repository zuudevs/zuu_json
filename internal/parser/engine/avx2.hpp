/**
 * @file avx2_engine.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief AVX2 implementation for Parser Backend
 * @version 1.5.0
 * @date 2026-06-29
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#ifdef __AVX2__
    #include "constants/simd.hpp"
    #include <bit>
    #include <cstring>
#endif

#include "parser/parser_base.hpp"
#include <cstdint>

namespace zuu::parser::engine {

template <typename LexerEngine>
class Avx2 : public ParserBase<Avx2<LexerEngine>, LexerEngine> {
  public:
    using ParserBase<Avx2<LexerEngine>, LexerEngine>::ParserBase;

#ifdef __AVX2__
    using block_t = __m256i;
    static inline constexpr uint8_t kSimdBlockSize = sizeof(block_t);
#endif // __AVX2__

    [[nodiscard]] std::string_view
        unescapeString(std::string_view src) noexcept {
        char* dest = this->res_.allocateStringBuffer(src.size());
        char* out = dest;
        const char* ptr = src.data();
        const char* end = ptr + src.size();

        while (ptr < end) {
#ifdef __AVX2__
            while (ptr + kSimdBlockSize <= end) {
                __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr));
                uint32_t mask =
                    _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, constants::simd32_esc));

                if (mask != constants::zero) {
                    auto offset = static_cast<uint32_t>(std::countr_zero(mask));
                    std::memcpy(out, ptr, offset);
                    out += offset;
                    ptr += offset;
                    break;
                }

                _mm256_storeu_si256(reinterpret_cast<__m256i*>(out), chunk);
                out += kSimdBlockSize;
                ptr += kSimdBlockSize;
            }
#endif // __AVX2__

            if (ptr >= end)
                break;

            if (*ptr == '\\') {
                ++ptr;
                if (ptr >= end) {
                    break;
                }

                switch (*ptr) {
                    case '\"':
                        *out++ = '\"';
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
                            this->status_ = core::JsonError::InvalidValue;
                            return {};
                        }
                        uint32_t cp = this->decodeUnicodeHex(ptr + 1);
                        ptr += constants::nibble;
                        if (cp >= 0xD800 && cp <= 0xDBFF) {
                            if (ptr + 6 <= end && ptr[1] == '\\' && ptr[2] == 'u') {
                                uint32_t cp2 = this->decodeUnicodeHex(ptr + 3);
                                if (cp2 >= 0xDC00 && cp2 <= 0xDFFF) {
                                    cp = 0x10000 +
                                         (((cp - 0xD800) << constants::digit) | (cp2 - 0xDC00));
                                    ptr += 6;
                                } else {
                                    this->status_ = core::JsonError::InvalidValue;
                                    return {};
                                }
                            } else {
                                this->status_ = core::JsonError::InvalidValue;
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
                            this->status_ = core::JsonError::InvalidValue;
                            return {};
                        }
                        break;
                    }
                    default:
                        this->status_ = core::JsonError::InvalidValue;
                        return {};
                }
            } else {
                *out++ = *ptr;
            }
            ++ptr;
        }
        return {dest, static_cast<uint64_t>(out - dest)};
    }
};

} // namespace zuu::parser::engine