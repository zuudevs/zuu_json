/**
 * @file avx2_engine.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief AVX2 specific implementation for Tokenizer Backend
 * @version 1.2.0
 * @date 2026-06-27
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#ifdef __AVX2__
#include <immintrin.h>
#endif

#include "tokenizer/tokenizer_base.hpp"
#include "utils/compiler.hpp"
#include <bit>
#include <cstdint>
#include "constants/simd.hpp"

namespace zuu::tokenizer {

class Avx2Engine : public TokenizerBase<Avx2Engine> {
  public:
    using TokenizerBase<Avx2Engine>::TokenizerBase;
    using block_t = __m256i;
    static inline constexpr uint8_t kBlockSize = sizeof(block_t);

  private:
#ifdef __AVX2__
    // --- Konstanta Algoritmik Tingkat Lanjut AVX2 ---

    // PSHUFB (Shuffle Byte) Lookup Table untuk mendeteksi Whitespace.
    // Memetakan 4-bit terbawah dari karakter ke karakter aslinya.
    alignas(32) static inline const __m256i lut_whitespace = _mm256_setr_epi8(
        0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x09, 0x0A, 0x00, 0x00, 0x0D, 0x00, 0x00,
        // AVX2 YMM register dibagi 2 lane 128-bit, jadi tabel harus diduplikasi
        0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x09, 0x0A, 0x00, 0x00, 0x0D, 0x00, 0x00
    );
    
    alignas(32) static inline const __m256i v_quote = _mm256_set1_epi8('"');
    alignas(32) static inline const __m256i v_escape = _mm256_set1_epi8('\\');
    alignas(32) static inline const __m256i v_31 = _mm256_set1_epi8(0x1F); // Batas kontrol
    alignas(32) static inline const __m256i v_zero = _mm256_setzero_si256();
#endif

  public:
    ZUU_HOT ZUU_ALWAYS_INLINE void skip_whitespace() noexcept {
#ifdef __AVX2__
        while (current_ + kBlockSize <= end_) {
            __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(current_));
            
            // --- PSHUFB Whitespace Magic (7 Instruksi -> 2 Instruksi) ---
            // _mm256_shuffle_epi8 menggunakan 4-bit bawah dari chunk sebagai indeks ke lut.
            // Jika chunk adalah salah satu whitespace, `mapped` akan memantulkan nilai chunk itu sendiri.
            __m256i mapped = _mm256_shuffle_epi8(lut_whitespace, chunk);
            __m256i match  = _mm256_cmpeq_epi8(mapped, chunk);
            
            uint32_t mask = _mm256_movemask_epi8(match);

            if (mask == 0xFFFFFFFF) {
                current_ += kBlockSize;
            } else {
                uint32_t non_ws_mask = ~mask;
                current_ += std::countr_zero(non_ws_mask);
                return;
            }
        }
#endif
        while (current_ < end_ && Lookup{}[*current_] == Lookup::Type::WhiteSpace) {
            ++current_;
        }
    }

    ZUU_HOT ZUU_ALWAYS_INLINE void read_string() noexcept {
        const char* begin = ++current_;
        const char* ptr = begin;
        bool has_escape = false;

#ifdef __AVX2__
        while (ptr + kBlockSize <= end_) {
            __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr));
            
            __m256i eq_quote = _mm256_cmpeq_epi8(chunk, v_quote);
            __m256i eq_esc   = _mm256_cmpeq_epi8(chunk, v_escape);
            
            // --- Saturated Subtraction Magic (3 Instruksi -> 2 Instruksi) ---
            // _mm256_subs_epu8 (Unsigned Saturated Subtraction).
            // Mengurangkan 31 (0x1F) dari chunk. Jika chunk <= 0x1F (Karakter Kontrol JSON),
            // hasilnya tidak akan menjadi minus (karena unsigned saturated), melainkan mentok di 0.
            __m256i sub_sat = _mm256_subs_epu8(chunk, v_31);
            __m256i is_ctrl = _mm256_cmpeq_epi8(sub_sat, v_zero);

            __m256i match = _mm256_or_si256(
                _mm256_or_si256(eq_quote, eq_esc), 
                is_ctrl
            );
            
            uint32_t mask = _mm256_movemask_epi8(match);

            if (mask != 0) {
                uint32_t byte_idx = std::countr_zero(mask);
                auto c = ptr[byte_idx];

                if (static_cast<uint8_t>(c) < 0x20) {
                    status_ = Error::UnescapedCharacter;
                    return;
                } else if (c == '"') {
                    res_.emplace_back(
                        Token::Type::String, 
                        std::string_view(begin, (ptr + byte_idx) - begin), 
                        has_escape
                    );

                    if (has_escape) {
                        hint_.string_escape_bytes_ += ((ptr + byte_idx) - begin);
                    }

                    current_ = ptr + byte_idx + 1;
                    return;
                } else { 
                    ptr += byte_idx;
                    break; 
                }
            }
            ptr += kBlockSize;
        }
#endif

        // Delegate ke Base Class untuk meminimalkan redundansi kode
        this->finish_string_scalar(ptr, begin, has_escape);
    }
};

} // namespace zuu::tokenizer