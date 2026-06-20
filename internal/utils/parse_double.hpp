/**
 * @file parse_double.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-17
 * 
 * @copyright Copyright (c) 2026
 */

#pragma once

#include <expected>
#include "traits/lookup_trait.hpp"
#include "traits/parser_trait.hpp"
#include "utils/strings.hpp"
#include "zuu_json/core/error.hpp"

namespace zuu::traits {

template <>
struct LookupTrait<double> {
	alignas(32) static inline constexpr double pow10_negative[] = {
		1e0,  
		1e-1,  
		1e-2,  
		1e-3,  
		1e-4,  
		1e-5,  
		1e-6,  
		1e-7,  
		1e-8,  
		1e-9,
		1e-10, 
		1e-11, 
		1e-12, 
		1e-13, 
		1e-14, 
		1e-15, 
		1e-16, 
		1e-17, 
		1e-18, 
		1e-19,
		1e-20, 
		1e-21, 
		1e-22
	};

	alignas(32) static inline constexpr double pow10_positive[] = {
		1e0, 
		1e1, 
		1e2, 
		1e3, 
		1e4, 
		1e5, 
		1e6, 
		1e7, 
		1e8, 
		1e9,
		1e10, 
		1e11, 
		1e12, 
		1e13, 
		1e14, 
		1e15, 
		1e16, 
		1e17, 
		1e18, 
		1e19,
		1e20, 
		1e21, 
		1e22
	};
};

template <>
struct ParserTrait<double> {
	using type = double;
    using Error = core::ParseError;
    using Result = std::expected<double, Error>;
    
    [[nodiscard]] inline constexpr Result
    operator()(const char* first, const char* last) const noexcept {
		if (first == last) {
            return std::unexpected{Error::InvalidFormat};
        }

        unsigned long long mantissa{};
        int decimal_shift{};
        bool is_negative{};
        bool has_digits{};

        if (*first == '-') { 
            is_negative = true; ++first; 
        }
        else if (*first == '+') {
            ++first; 
        }

        while (first != last && utils::is_numeric(*first)) {
            has_digits = true;
            if (mantissa < 1844674407370955161ULL) [[likely]] {
                mantissa = mantissa * constants::digit + (*first - '0');
            } else {
                decimal_shift++;
            }
            ++first;
        }

        if (first != last && *first == '.') {
            ++first;
            while (first != last && utils::is_numeric(*first)) {
                has_digits = true;
                if (mantissa < 1844674407370955161ULL) [[likely]] {
                    mantissa = mantissa * constants::digit + (*first - '0');
                    decimal_shift--; 
                }
                ++first;
            }
        }

        if (!has_digits) {
            return std::unexpected{Error::InvalidFormat};
        }

        if (first != last && (*first == 'e' || *first == 'E')) {
            const char* exp_ptr = first;
            ++first;

            if (first != last) {
                bool exp_negative = (*first == '-');
                if (*first == '-' || *first == '+') ++first;

                bool has_exp_digits = false;
                int exponent = 0;
                while (first != last && utils::is_numeric(*first)) {
                    has_exp_digits = true;
                    // Mencegah overflow pada exponent
                    if (exponent < 1000000) {
                        exponent = exponent * constants::digit + (*first - '0');
                    }
                    ++first;
                }

                if (has_exp_digits) {
                    if (exp_negative) decimal_shift -= exponent;
                    else decimal_shift += exponent;
                } else {
                    first = exp_ptr; 
                }
            } else {
                first = exp_ptr;
            }
        }

        auto result = static_cast<type>(mantissa);

        if (decimal_shift < 0) {
            int neg_shift = -decimal_shift;
            if (neg_shift <= 22) result *= traits::LookupTrait<type>::pow10_negative[neg_shift];
            else {
                while(neg_shift > 22) { 
                    result *= 0.1; 
                    neg_shift--; 
                }
                result *= traits::LookupTrait<type>::pow10_negative[22];
            }
        } else if (decimal_shift > 0) {
            if (decimal_shift <= 22) {
                result *= traits::LookupTrait<type>::pow10_positive[decimal_shift];
            }
            else {
                while(decimal_shift > 22) { 
                    result *= 10.0; decimal_shift--; 
                }
                result *= traits::LookupTrait<type>::pow10_positive[22];
            }
        }

        if (is_negative) {
            result = -result;
        }

        return result;
    }
};

} // namespace zuu