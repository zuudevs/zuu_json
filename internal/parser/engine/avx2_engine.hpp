/**
 * @file avx2_engine.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief AVX2 implementation for Parser Backend
 * @version 1.0.0
 * @date 2026-06-27
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "parser/parser_base.hpp"
#include "utils/swar.hpp"
#include <cstdint>
#include <cstring>
#include <bit>

#ifdef __AVX2__
#include <immintrin.h>
#endif

namespace zuu::parser {

class Avx2Engine : public ParserBase<Avx2Engine> {
  public:
    using ParserBase<Avx2Engine>::ParserBase;
	static inline constexpr uint8_t kSwarBlockSize = sizeof(uint64_t);
#ifdef __AVX2__
	using block_t = __m256i;
	static inline constexpr uint8_t kSimdBlockSize = sizeof(block_t);

	static inline const block_t simd32_escape = _mm256_set1_epi8('\\');
#endif // __AVX2__

    [[nodiscard]] std::string_view unescapeString(std::string_view src) noexcept {
        char* dest = this->res_.allocateStringBuffer(src.size());
        char* out = dest;
        const char* ptr = src.data();
        const char* end = ptr + src.size();

        while (ptr < end) {
#ifdef __AVX2__
            while (ptr + kSimdBlockSize <= end) {
                __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr));
                __m256i match = _mm256_cmpeq_epi8(chunk, simd32_escape);
                uint32_t mask = _mm256_movemask_epi8(match);

                if (mask != 0) {
                    auto offset = static_cast<uint32_t>(std::countr_zero(mask));
                    std::memcpy(out, ptr, offset);
                    out += offset;
                    ptr += offset;
                    goto handle_escape;
                }

                _mm256_storeu_si256(reinterpret_cast<__m256i*>(out), chunk);
                out += kSimdBlockSize;
                ptr += kSimdBlockSize;
            }
#endif // __AVX2__

            while (ptr + kSwarBlockSize <= end) {
                uint64_t block{};
                std::memcpy(&block, ptr, sizeof(uint64_t));

                uint64_t mask = utils::find_zero_byte_mask(block ^ constants::swar8_escape);
                if (mask != 0) {
                    auto offset = static_cast<uint32_t>(std::countr_zero(mask) >> 3);
                    std::memcpy(out, ptr, offset);
                    out += offset;
                    ptr += offset;
                    goto handle_escape;
                }

                std::memcpy(out, ptr, sizeof(uint64_t));
                out += kSwarBlockSize;
                ptr += kSwarBlockSize;
            }

            if (ptr >= end) {
                break; 
            }

        handle_escape:
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
                            this->status_ = core::JsonError::InvalidValue;
                            return {};
                        }
                        
                        uint32_t cp = this->decodeUnicodeHex(ptr + 1);
                        ptr += constants::nibble;
                        if (cp >= 0xD800 && cp <= 0xDBFF) {
                            if (ptr + 6 <= end && ptr[1] == '\\' && ptr[2] == 'u') {
                                uint32_t cp2 = this->decodeUnicodeHex(ptr + 3);
                                if (cp2 >= 0xDC00 && cp2 <= 0xDFFF) {
                                    cp = 0x10000 + (((cp - 0xD800) << constants::digit) | (cp2 - 0xDC00));
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

} // namespace zuu::parser