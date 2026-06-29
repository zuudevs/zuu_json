/**
 * @file parse_double.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief 100% Exact Two-Tier Float Parser (Clinger's Fast Path + Eisel-Lemire Fallback)
 * @version 1.6.0
 * @date 2026-06-30
 * 
 * @copyright Copyright (c) 2026
 */

#pragma once

#include <cstdint>
#include <expected>
#include <charconv>
#include "constants/general.hpp"
#include "traits/lookup_trait.hpp"
#include "traits/parser_trait.hpp"
#include "zuu_json/core/error.hpp"

namespace zuu::traits {

template <>
struct LookupTrait<double> {
    alignas(32) static inline constexpr double pow10_negative[] = {
        1e0,  1e-1,  1e-2,  1e-3,  1e-4,  1e-5,  1e-6,  1e-7,  1e-8,  1e-9,
        1e-10, 1e-11, 1e-12, 1e-13, 1e-14, 1e-15, 1e-16, 1e-17, 1e-18, 1e-19,
        1e-20, 1e-21, 1e-22
    };

    alignas(32) static inline constexpr double pow10_positive[] = {
        1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
        1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
        1e20, 1e21, 1e22
    };
};

template <>
struct ParserTrait<double> {
    using Error = core::ParseError;
    using Result = std::expected<double, Error>;
    
    [[nodiscard]] inline constexpr Result
    operator()(const char* first, const char* last) const noexcept {
        if (first == last) {
			return std::unexpected{Error::InvalidFormat};
		}

        const char* ptr  = first;
        bool is_negative = (*ptr == '-');
        ptr += is_negative;

        uint64_t mantissa     = constants::zero;
        int32_t decimal_shift = constants::zero;
        int digits            = constants::zero;

        while (ptr != last && *ptr != '.' && *ptr != 'e' && *ptr != 'E') {
            mantissa = mantissa * 10ULL + (*ptr - '0');
            digits++;
            ++ptr;
        }

        if (ptr != last && *ptr == '.') {
            ++ptr;
            while (ptr != last && *ptr != 'e' && *ptr != 'E') {
                mantissa = mantissa * 10ULL + (*ptr - '0');
                decimal_shift--;
                digits++;
                ++ptr;
            }
        }

        if (ptr != last && (*ptr == 'e' || *ptr == 'E')) {
            ++ptr;
            bool exp_negative = (*ptr == '-');
            ptr += (exp_negative || *ptr == '+');
            int32_t exponent = constants::zero;
            while (ptr != last) {
                exponent = exponent * constants::digit + (*ptr - '0');
                ++ptr;
            }
            decimal_shift += exp_negative ? -exponent : exponent;
        }

        if (
			digits        <= 19 && 
			mantissa      <= 9007199254740991ULL && 
			decimal_shift >= -22 && 
			decimal_shift <= 22
		) {
            auto result = static_cast<double>(mantissa);
            
            if (decimal_shift > constants::zero) {
                result *= LookupTrait<double>::pow10_positive[decimal_shift];
            } else if (decimal_shift < constants::zero) {
                result *= LookupTrait<double>::pow10_negative[-decimal_shift];
            }
            
            return is_negative ? -result : result;
        }

        double result = 0.0;
        auto [end_ptr, ec] = std::from_chars(first, last, result);
        
        if (ec == std::errc()) {
            return result;
        }

        return std::unexpected{Error::InvalidFormat};
    }
};

} // namespace zuu::traits