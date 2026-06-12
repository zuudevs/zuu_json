/**
 * @file scalar_storage.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-06
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

namespace zuu::models {

union ScalarStorage {
  public:
    bool b;
    long long i;
    long double d;
    unsigned long long index;

    constexpr ScalarStorage(bool value) noexcept : b(value) {}
    constexpr ScalarStorage(long long value) noexcept : i(value) {}
    constexpr ScalarStorage(long double value) noexcept : d(value) {}
    constexpr ScalarStorage(unsigned long long index) noexcept : index(index) {}
};

} // namespace zuu::models