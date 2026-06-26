/**
 * @file avx2_backend.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief SWAR specific implementation for Tokenizer Backend
 * @version 1.0.0
 * @date 2026-06-26
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

/**
 * @brief Implementasi backend Tokenizer menggunakan SWAR (SIMD Within A Register).
 */
class Avx2Engine : public TokenizerBase<Avx2Engine> {
  public:
    using TokenizerBase<Avx2Engine>::TokenizerBase;
	using block_t = __m256i;
	static inline constexpr uint8_t kBlockSize = sizeof(block_t);

    ZUU_HOT ZUU_ALWAYS_INLINE void skip_whitespace() noexcept {
        // Buat vektor YMM yang berisi 32 karakter whitespace identik
        const __m256i w_space = _mm256_set1_epi8(' ');
        const __m256i w_nl    = _mm256_set1_epi8('\n');
        const __m256i w_cr    = _mm256_set1_epi8('\r');
        const __m256i w_tab   = _mm256_set1_epi8('\t');

        while (current_ + kBlockSize <= end_) {
            // Load 32 byte karakter JSON dari memori (unaligned load)
            __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(current_));
            
            // Bandingkan vektor chunk dengan setiap jenis whitespace (akan menghasilkan 0xFF jika sama)
            __m256i eq_space = _mm256_cmpeq_epi8(chunk, w_space);
            __m256i eq_nl    = _mm256_cmpeq_epi8(chunk, w_nl);
            __m256i eq_cr    = _mm256_cmpeq_epi8(chunk, w_cr);
            __m256i eq_tab   = _mm256_cmpeq_epi8(chunk, w_tab);

            // Gabungkan semua hasil perbandingan menggunakan bitwise OR
            __m256i match = _mm256_or_si256(
                _mm256_or_si256(eq_space, eq_nl), 
                _mm256_or_si256(eq_cr, eq_tab)
            );
            
            // Ekstrak bit paling signifikan (MSB) dari setiap byte menjadi 32-bit integer mask
            uint32_t mask = _mm256_movemask_epi8(match);

            if (mask == 0xFFFFFFFF) {
                // Seluruh 32 byte adalah whitespace, lompat jauh!
                current_ += kBlockSize;
            } else {
                // Ada karakter non-whitespace. Cari tahu lokasinya.
                uint32_t non_ws_mask = ~mask;
                current_ += std::countr_zero(non_ws_mask);
                return; // Langsung keluar karena sudah di posisi non-whitespace
            }
        }

        // Fallback (scalar) untuk memproses sisa memori jika < 32 bytes
        while (current_ < end_ && Lookup{}[*current_] == Lookup::Type::WhiteSpace) {
            ++current_;
        }
    }

    ZUU_HOT ZUU_ALWAYS_INLINE void read_string() noexcept {
        const char* begin = ++current_;
        const char* ptr = begin;
        const char* end = end_;
        bool has_escape = false;

        while (ptr + kBlockSize <= end) {
            const char* begin = ++current_;
			const char* ptr = begin;
			bool has_escape = false;

			const __m256i quote  = _mm256_set1_epi8('"');
			const __m256i escape = _mm256_set1_epi8('\\');
			
			// Konstanta untuk deteksi Control Characters (0x00 - 0x1F) secara aman
			const __m256i space  = _mm256_set1_epi8(0x20);
			const __m256i zero   = _mm256_set1_epi8(-1); 

			while (ptr + kBlockSize <= end_) {
				__m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr));
				
				__m256i eq_quote = _mm256_cmpeq_epi8(chunk, quote);
				__m256i eq_esc   = _mm256_cmpeq_epi8(chunk, escape);
				
				// AVX2 membandingkan signed integer (byte >= 128 akan dianggap negatif).
				// Control char adalah byte yang: (chunk < 0x20) DAN (chunk > -1).
				__m256i lt_space = _mm256_cmpgt_epi8(space, chunk);
				__m256i gt_neg1  = _mm256_cmpgt_epi8(chunk, zero);
				__m256i is_ctrl  = _mm256_and_si256(lt_space, gt_neg1);

				// Gabungkan masker kutip, backslash, dan kontrol
				__m256i match = _mm256_or_si256(
					_mm256_or_si256(eq_quote, eq_esc), 
					is_ctrl
				);
				
				uint32_t mask = _mm256_movemask_epi8(match);

				if (mask != 0) {
					// Ketemu tanda kutip penutup, backslash, atau error control char!
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
						// Ada backslash escape (\\). 
						// Kita keluar dari AVX2 dan biarkan scalar logic memproses escape sequence.
						ptr += byte_idx;
						break; 
					}
				}
				ptr += kBlockSize;
			}

			// Fallback scalar untuk escape sequences dan sisa data string
			while (ptr < end_) {
				char c = *ptr;
				if (static_cast<unsigned char>(c) < 0x20) {
					status_ = Error::UnescapedCharacter;
					return;
				}

				if (c == '\"') {
					res_.emplace_back(
						Token::Type::String, 
						std::string_view(begin, ptr - begin), 
						has_escape
					);

					if (has_escape) {
						hint_.string_escape_bytes_ += (ptr - begin);
					}

					current_ = ptr + 1;
					return;
				}
				if (c == '\\') [[unlikely]] {
					has_escape = true;
					ptr += 2;
					if (ptr > end_) {
						status_ = core::JsonError::InvalidValue;
						return;
					}
					continue;
				}
				++ptr;
			}

			status_ = core::JsonError::InvalidValue;
		}
	}
};

} // namespace zuu::tokenizer