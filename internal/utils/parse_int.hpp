/**
 * @file parse_int.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-18
 * 
 * @copyright Copyright (c) 2026
 */

#pragma once 

#include "constants/swar.hpp"
#include <bit>
#include <cstdint>
#include <cstring>
#include <expected>
#include "traits/parser_trait.hpp"
#include "utils/strings.hpp"
#include "zuu_json/core/error.hpp"

namespace zuu::traits {

template <>
struct ParserTrait<long long> {
    using Error = core::ParseError;
    using Result = std::expected<uint64_t, Error>;
    
    [[nodiscard]] inline constexpr Result
    operator()(const char* first, const char* last) const noexcept {
        if (first == last) {
            return std::unexpected{Error::InvalidFormat};
        }

        uint64_t value{};
        bool is_negative{};
        bool has_digits{};

        if (first < last && *first == '-') {
            is_negative = true;
            ++first;
        }

        while (first + sizeof(uint64_t) <= last) {
            uint64_t block{};
            std::memcpy(&block, first, sizeof(uint64_t));

            uint64_t val = block - constants::swar8_zero;
            uint64_t non_digits = ((val + constants::swar8_digit_bias) | val) & constants::swar8_msb;

            if (non_digits == 0) [[likely]] {
                has_digits = true;
                uint64_t d = val & 0x0F0F0F0F0F0F0F0FULL;
                d = (d * 10) + (d >> 8);
                d = (d & 0x00FF00FF00FF00FFULL) * 100 + ((d >> 16) & 0x00FF00FF00FF00FFULL);
                d = (d & 0x0000FFFF0000FFFFULL) * 10000 + ((d >> 32) & 0x0000FFFF0000FFFFULL);
                value = value * 100000000ULL + static_cast<uint32_t>(d);
                first += sizeof(uint64_t);
            } else {
                uint32_t valid_len = std::countr_zero(non_digits) >> 3;
                if (valid_len == 0) {
                    break;
                }
                has_digits = true;
                uint64_t shift_bits = (8 - valid_len) * 8;
                val <<= shift_bits;

                uint64_t d = val & 0x0F0F0F0F0F0F0F0FULL;
                d = (d * 10) + (d >> 8);
                d = (d & 0x00FF00FF00FF00FFULL) * 100 + ((d >> 16) & 0x00FF00FF00FF00FFULL);
                d = (d & 0x0000FFFF0000FFFFULL) * 10000 + ((d >> 32) & 0x0000FFFF0000FFFFULL);

                static constexpr uint64_t pow10[8] = {
                    1, 10, 100, 1000, 10000, 100000, 1000000, 10000000
                };

                value = value * pow10[valid_len] + static_cast<uint32_t>(d);
                first += valid_len;
                break; 
            }
        }

        while (first < last && utils::is_numeric(*first)) {
            has_digits = true;
            value = value * constants::digit + (*first - '0');
            ++first;
        }

        if (!has_digits || first != last) {
            return std::unexpected{Error::InvalidFormat};
        }

        return Result{(is_negative ? -static_cast<uint64_t>(value) : static_cast<uint64_t>(value))};
    }
};

} // namespace zuu::traits