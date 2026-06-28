/**
 * @file avx2_engine.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief AVX2 implementation for Parser Backend (Dead Branch Pruned)
 * @version 1.4.0
 * @date 2026-06-28
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "parser/parser_base.hpp"
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

#ifdef __AVX2__
        while (ptr + kSimdBlockSize <= end) {
            __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr));
            uint32_t mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, simd32_escape));

            if (mask != 0) {
                auto offset = static_cast<uint32_t>(std::countr_zero(mask));
                std::memcpy(out, ptr, offset);
                out += offset;
                ptr += offset;
                
                // Lempar ke fallback skalar sampai akhir string
                this->finish_string_scalar(out, ptr, end);
                if (this->has_error()) return {};
                return {dest, static_cast<uint64_t>(out - dest)};
            }

            _mm256_storeu_si256(reinterpret_cast<__m256i*>(out), chunk);
            out += kSimdBlockSize;
            ptr += kSimdBlockSize;
        }
#endif // __AVX2__

        // Sisa byte skalar (ekor) atau string tanpa escape di bawah 32 bytes
        this->finish_string_scalar(out, ptr, end);
        if (this->has_error()) {
			return {};
		}
        
        return {dest, static_cast<uint64_t>(out - dest)};
    }
};

} // namespace zuu::parser