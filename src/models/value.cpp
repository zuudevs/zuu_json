/**
 * @file value.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-16
 *
 * @copyright Copyright (c) 2026
 */

#include "zuu_json/models/value.hpp"
#include "models/storage.hpp"
#include <algorithm>

namespace zuu::models {

Value::Value(const Storage* storage, JsonValue value) noexcept
    : storage_(storage)
    , value_(value) {}

Value Value::fromInternal(const Storage* storage, const JsonValue& v) noexcept {
    return Value(storage, v);
}

Value Value::createNull(const Storage* storage) noexcept {
    return Value(storage, JsonValue::Null());
}

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

// ── Strict Extraction ──

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
    // Toleransi: Integer dapat dibaca sebagai double secara strict
    if (value_.get_type() == Type::Integer)
        return static_cast<long double>(value_.as_integer());
    if (value_.get_type() != Type::Double)
        return std::unexpected{core::JsonError::InvalidType};
    return value_.as_double();
}

Value::Result<std::string_view> Value::as_string() const noexcept {
    if (value_.get_type() != Type::String)
        return std::unexpected{core::JsonError::InvalidType};
    return storage_->string(value_.as_index());
}

// ── Fluent / Default Extraction ──

bool Value::get_bool(bool default_val) const noexcept {
    if (value_.get_type() != Type::Boolean)
        return default_val;
    return value_.as_bool();
}

long long Value::get_integer(long long default_val) const noexcept {
    if (value_.get_type() != Type::Integer)
        return default_val;
    return value_.as_integer();
}

double Value::get_double(double default_val) const noexcept {
    if (value_.get_type() == Type::Integer)
        return static_cast<double>(value_.as_integer());
    if (value_.get_type() != Type::Double)
        return default_val;
    return static_cast<double>(value_.as_double());
}

std::string_view Value::get_string(std::string_view default_val) const noexcept {
    if (value_.get_type() != Type::String)
        return default_val;
    return storage_->string(value_.as_index());
}

// ── Container & Traversal ──

size_t Value::size() const noexcept {
    if (value_.get_type() == Type::Array)
        return storage_->array(value_.as_index()).size();
    if (value_.get_type() == Type::Object)
        return storage_->object(value_.as_index()).size();
    return 0;
}

bool Value::contains(std::string_view key) const noexcept {
    if (value_.get_type() != Type::Object)
        return false;

    const auto obj = storage_->object(value_.as_index());
    auto it = std::ranges::lower_bound(
        obj, 
		key, 
		{}, 
		[this](const JsonMember& member) {
            return storage_->string(member.key_index_);
        }
	);

    return (it != obj.end() && storage_->string(it->key_index_) == key);
}

// Strict at()
Value::Result<Value> Value::at(size_t index) const noexcept {
    if (value_.get_type() != Type::Array) {
        return std::unexpected{core::JsonError::IsNotArray};
    }
    const auto arr = storage_->array(value_.as_index());
    if (index >= arr.size()) {
        return std::unexpected{core::JsonError::InvalidValue};
    }
    return fromInternal(storage_, arr[index]);
}

Value::Result<Value> Value::at(std::string_view key) const noexcept {
    if (value_.get_type() != Type::Object) {
        return std::unexpected{core::JsonError::IsNotObject};
    }

    const auto obj = storage_->object(value_.as_index());
    auto it = std::ranges::lower_bound(
        obj, 
		key, 
		{}, 
		[this](const JsonMember& member) {
            return storage_->string(member.key_index_);
        }
	);

    if (it != obj.end() && storage_->string(it->key_index_) == key) {
        return fromInternal(storage_, it->value_);
    }

    return std::unexpected{core::JsonError::InvalidValue};
}

// Fluent operator[] (Optional Chaining safe)
Value Value::operator[](size_t index) const noexcept {
    auto res = at(index);
    if (res)
        return res.value();
    return createNull(storage_);
}

Value Value::operator[](std::string_view key) const noexcept {
    auto res = at(key);
    if (res)
        return res.value();
    return createNull(storage_);
}

} // namespace zuu::models