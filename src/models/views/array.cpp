/**
 * @file array.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-21
 *
 * @copyright Copyright (c) 2026
 */

#include "models/views/array.hpp"
#include "allocators/storage.hpp"
#include "zuu_json/models/value.hpp"

namespace zuu::models::views {

Value
    Array::Iterator::operator*() const noexcept {
    return Value::fromInternal(storage_, *ptr_);
}

Value
    Array::Iterator::operator[](difference_type n) const noexcept {
    return Value::fromInternal(storage_, *(ptr_ + n));
}

Value
    Array::operator[](std::size_t index) const noexcept {
    return Value::fromInternal(storage_, span_[index]);
}

} // namespace zuu::models::views