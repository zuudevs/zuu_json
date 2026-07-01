/**
 * @file member_view.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-21
 * 
 * @copyright Copyright (c) 2026
 */

#include "models/views/member.hpp"
#include "zuu_json/models/value.hpp"

namespace zuu::models::views {

Value Member::value() const noexcept {
    return Value::fromInternal(storage_, val_);
}

} // namespace zuu::models::views