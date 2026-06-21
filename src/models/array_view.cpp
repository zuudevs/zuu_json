/**
 * @file array_view.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-21
 * 
 * @copyright Copyright (c) 2026
 */

#include "models/array_view.hpp"
#include "zuu_json/models/value.hpp"

namespace zuu::models {

Value ArrayView::Iterator::operator*() const noexcept {
    return Value::fromInternal(storage_, *ptr_);
}

Value ArrayView::Iterator::operator[](difference_type n) const noexcept {
    return Value::fromInternal(storage_, *(ptr_ + n));
}

Value ArrayView::operator[](std::size_t index) const noexcept {
    return Value::fromInternal(storage_, span_[index]);
}

} // namespace zuu::models