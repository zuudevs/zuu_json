/**
 * @file value.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-06
 *
 * @copyright Copyright (c) 2026
 */

#include "zuu_json/models/value.hpp"
#include "models/storage.hpp"

namespace zuu::models {

Value::Value(const Storage* storage, JsonValue value) noexcept
    : storage_(storage)
    , value_(value) {}

// ── Internal factory ──────────────────────────────────────────────────────────

Value Value::fromInternal(const Storage* storage, const JsonValue& v) noexcept {
    return Value(storage, v);
}

// ── Type inspection ───────────────────────────────────────────────────────────

Value::Type Value::type() const noexcept {
    return value_.get_type();
}
bool Value::is_null() const noexcept {
    return value_.get_type() == Type::Null;
}
bool Value::is_bool() const noexcept {
    return value_.get_type() == Type::Boolean;
}
bool Value::is_integer() const noexcept {
    return value_.get_type() == Type::Integer;
}
bool Value::is_double() const noexcept {
    return value_.get_type() == Type::Double;
}
bool Value::is_string() const noexcept {
    return value_.get_type() == Type::String;
}
bool Value::is_array() const noexcept {
    return value_.get_type() == Type::Array;
}
bool Value::is_object() const noexcept {
    return value_.get_type() == Type::Object;
}

// ── Typed value extraction ────────────────────────────────────────────────────

Value::Result<bool> Value::as_bool() const noexcept {
    if (value_.get_type() != Type::Boolean)
        return std::unexpected{core::JsonError::InvalidType};
    return value_.as_bool();
}

Value::Result<long long> Value::as_integer() const noexcept {
    if (value_.get_type() != Type::Integer)
        return std::unexpected{core::JsonError::InvalidType};
    return value_.as_integer();
}

Value::Result<long double> Value::as_double() const noexcept {
    if (value_.get_type() != Type::Double)
        return std::unexpected{core::JsonError::InvalidType};
    return value_.as_double();
}

Value::Result<std::string_view> Value::as_string() const noexcept {
    if (value_.get_type() != Type::String)
        return std::unexpected{core::JsonError::InvalidType};
    return storage_->string(value_.as_index());
}

// ── Container access ──────────────────────────────────────────────────────────

size_t Value::size() const noexcept {
    if (value_.get_type() == Type::Array)
        return storage_->array(value_.as_index()).size();
    if (value_.get_type() == Type::Object)
        return storage_->object(value_.as_index()).size();
    return 0;
}

Value::Result<Value> Value::operator[](size_t index) const noexcept {
    if (value_.get_type() != Type::Array) {
        return std::unexpected{core::JsonError::IsNotArray};
    }
    const auto& arr = storage_->array(value_.as_index());
    if (index >= arr.size()) {
        return std::unexpected{core::JsonError::InvalidValue};
    }
    return fromInternal(storage_, arr[index]);
}

Value::Result<Value> Value::operator[](std::string_view key) const noexcept {
    if (value_.get_type() != Type::Object) {
        return std::unexpected{core::JsonError::IsNotObject};
    }
    const auto& obj = storage_->object(value_.as_index());
    for (const auto& member : obj) {
        if (storage_->string(member.key_index_) == key) {
            return fromInternal(storage_, member.value_);
        }
    }
    return std::unexpected{core::JsonError::InvalidValue};
}

bool Value::contains(std::string_view key) const noexcept {
    if (value_.get_type() != Type::Object)
        return false;
    const auto& obj = storage_->object(value_.as_index());
    for (const auto& member : obj) {
        if (storage_->string(member.key_index_) == key) {
            return true;
        }
    }
    return false;
}

} // namespace zuu::models