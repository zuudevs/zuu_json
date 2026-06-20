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

#include <cstdint>
#include <expected>
#include "constants/general.hpp"
#include "traits/lookup_trait.hpp"
#include "traits/parser_trait.hpp"
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
    using Error = core::ParseError;
    using Result = std::expected<double, Error>;
    
    [[nodiscard]] inline constexpr Result
    operator()(const char* first, const char* last) const noexcept {
		if (first == last) {
            return std::unexpected{Error::InvalidFormat};
        }

        uint64_t mantissa{};
        int32_t decimal_shift{};
        bool is_negative = (*first == '-');
        first += is_negative;
        while (first != last && *first != '.' && *first != 'e' && *first != 'E') {
            mantissa = mantissa * constants::digit + (*first - '0');
            ++first;
        }

        if (first != last && *first == '.') {
            ++first;
            while (first != last && *first != 'e' && *first != 'E') {
                mantissa = mantissa * constants::digit + (*first - '0');
                decimal_shift++;
                ++first;
            }
        }

        if (first != last && (*first == 'e' || *first == 'E')) {
            ++first;

            bool exp_negative = (*first == '-');
            first += (exp_negative || *first == '+'); 
            int32_t exponent = constants::zero;
            while (first != last) {
                exponent = exponent * constants::digit + (*first - '0');
                ++first;
            }

            if (exp_negative) {
				decimal_shift += exponent;
			} else {
				decimal_shift -= exponent;
			}
        }

        auto result = static_cast<double>(mantissa);

        if (decimal_shift > constants::zero) {
            if (decimal_shift <= 22) {
				result *= traits::LookupTrait<double>::pow10_negative[decimal_shift];
			} else {
                while(decimal_shift > 22) { 
                    result *= 0.1; 
                    decimal_shift--; 
                }
                result *= traits::LookupTrait<double>::pow10_negative[22];
            }
        } else if (decimal_shift < constants::zero) {
            int pos_shift = -decimal_shift;
            if (pos_shift <= 22) {
                result *= traits::LookupTrait<double>::pow10_positive[pos_shift];
            } else {
                while(pos_shift > 22) { 
                    result *= 10.0; pos_shift--; 
                }
                result *= traits::LookupTrait<double>::pow10_positive[22];
            }
        }

        if (is_negative) {
            result = -result;
        }

        return result;
    }
};

} // namespace zuu