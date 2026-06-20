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

#include "constants/general.hpp"
#include <cstdint>
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