/**
 * @file member_view.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-21
 *
 * @copyright Copyright (c) 2026
 */

#include "models/views/object.hpp"

namespace zuu::models::views {

Member
    Object::Iterator::operator*() const noexcept {
    return Member{.key_ = storage_->resolveKey(*ptr_), .storage_ = storage_, .val_ = ptr_->value_};
}

Member
    Object::Iterator::operator[](difference_type n) const noexcept {
    const auto* target = ptr_ + n;
    return Member{
        .key_ = storage_->resolveKey(*target), .storage_ = storage_, .val_ = target->value_};
}

} // namespace zuu::models::views