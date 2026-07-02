/**
 * @file parser.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-18
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "utils/parse_double.hpp"
#include "utils/parse_int.hpp"

namespace zuu::utils {

template <typename T>
[[nodiscard]] inline constexpr auto
    parse(const char* first, const char* last) noexcept {
    return traits::ParserTrait<T>{}(first, last);
}

template <typename T>
[[nodiscard]] inline constexpr auto
    parse(std::string_view value) noexcept {
    return traits::ParserTrait<T>{}(value.data(), value.data() + value.size());
}

} // namespace zuu::utils