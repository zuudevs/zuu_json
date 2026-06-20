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
#include "utils/strings.hpp"
#include <algorithm>

namespace zuu::models {

static constexpr JsonValue kNullValue = JsonValue::Null();

Value::Value(const Storage* storage, const JsonValue* value) noexcept
    : storage_(storage)
    , value_(value) {}

Value Value::fromInternal(const Storage* storage, const JsonValue& v) noexcept {
    return Value(storage, &v);
}

Value Value::createNull(const Storage* storage) noexcept {
    return Value(storage, &kNullValue);
}

Value::Type Value::type() const noexcept {
    return value_->get_type();
}
bool Value::is_null() const noexcept {
    return value_->get_type() == Type::Null;
}
bool Value::is_bool() const noexcept {
    return value_->get_type() == Type::Boolean;
}
bool Value::is_integer() const noexcept {
    return value_->get_type() == Type::Integer;
}
bool Value::is_double() const noexcept {
    return value_->get_type() == Type::Double;
}
bool Value::is_string() const noexcept {
    return value_->get_type() == Type::String || value_->get_type() == Type::ShortString;
}
bool Value::is_array() const noexcept {
    return value_->get_type() == Type::Array;
}
bool Value::is_object() const noexcept {
    return value_->get_type() == Type::Object;
}

// ── Strict Extraction ──

Value::Result<bool> Value::as_bool() const noexcept {
    if (value_->get_type() != Type::Boolean)
        return std::unexpected{core::JsonError::InvalidType};
    return value_->as_bool();
}

Value::Result<long long> Value::as_integer() const noexcept {
    if (value_->get_type() != Type::Integer)
        return std::unexpected{core::JsonError::InvalidType};
    return value_->as_integer();
}

Value::Result<long double> Value::as_double() const noexcept {
    if (value_->get_type() == Type::Integer)
        return static_cast<long double>(value_->as_integer());
    if (value_->get_type() != Type::Double)
        return std::unexpected{core::JsonError::InvalidType};
    return value_->as_double();
}

Value::Result<std::string_view> Value::as_string() const noexcept {
    if (value_->get_type() == Type::String)
        return storage_->string(value_->as_index());
        
    if (value_->get_type() == Type::ShortString) {
        const char* chars = reinterpret_cast<const char*>(&value_->data_);
        size_t len = (value_->get_payload() >> 40) & constants::uint8_max;
        return std::string_view(chars, len);
    }
    
    return std::unexpected{core::JsonError::InvalidType};
}

// ── Fluent / Default Extraction ──

bool Value::get_bool(bool default_val) const noexcept {
    if (value_->get_type() != Type::Boolean)
        return default_val;
    return value_->as_bool();
}

long long Value::get_integer(long long default_val) const noexcept {
    if (value_->get_type() != Type::Integer)
        return default_val;
    return value_->as_integer();
}

double Value::get_double(double default_val) const noexcept {
    if (value_->get_type() == Type::Integer)
        return static_cast<double>(value_->as_integer());
    if (value_->get_type() != Type::Double)
        return default_val;
    return static_cast<double>(value_->as_double());
}

std::string_view Value::get_string(std::string_view default_val) const noexcept {
    if (value_->get_type() == Type::String)
        return storage_->string(value_->as_index());
        
    if (value_->get_type() == Type::ShortString) {
        const char* chars = reinterpret_cast<const char*>(&value_->data_);
        size_t len = (value_->get_payload() >> 40) & constants::uint8_max;
        return {chars, len};
    }
    
    return default_val;
}

// ── Container & Traversal ──

unsigned long long Value::size() const noexcept {
    if (value_->get_type() == Type::Array)
        return storage_->array(value_->as_index()).size();
    if (value_->get_type() == Type::Object)
        return storage_->object(value_->as_index()).size();
    return 0;
}

bool Value::contains(std::string_view key) const noexcept {
    if (value_->get_type() != Type::Object)
        return false;

    const auto obj = storage_->object(value_->as_index());
    const unsigned long long target_prefix = utils::build_string_prefix(key);
    
    if (obj.size() <= constants::word) {
        auto it = std::ranges::find_if(
			obj, 
			[this, key, target_prefix](const JsonMember& member) {
				if ((member.key_index_ >> constants::dword) != target_prefix) return false;
            	return storage_->string(member.key_index_ & 0xFFFFFFFFULL) == key;
			}
		);
        return it != obj.end();
    } else {
        auto it = std::lower_bound(
			obj.begin(), 
			obj.end(), 
			key, 
			[this, target_prefix](const JsonMember& member, std::string_view k) {
            	const unsigned long long member_prefix = member.key_index_ >> constants::dword;
            	if (member_prefix != target_prefix) {
					return member_prefix < target_prefix;
				}
            	return storage_->string(member.key_index_ & 0xFFFFFFFFULL) < k;
    	    }
		);
        return (
			it != obj.end() && 
			(it->key_index_ >> constants::dword) == target_prefix && 
			storage_->string(it->key_index_ & 0xFFFFFFFFULL) == key
		);
    }
}

// Strict at()
Value::Result<Value> Value::at(unsigned long long index) const noexcept {
    if (value_->get_type() != Type::Array) {
        return std::unexpected{core::JsonError::IsNotArray};
    }
    const auto arr = storage_->array(value_->as_index());
    if (index >= arr.size()) {
        return std::unexpected{core::JsonError::InvalidValue};
    }
    return fromInternal(storage_, arr[index]);
}

Value::Result<Value> Value::at(std::string_view key) const noexcept {
    if (value_->get_type() != Type::Object) {
        return std::unexpected{core::JsonError::IsNotObject};
    }

    const auto obj = storage_->object(value_->as_index());
    const unsigned long long target_prefix = utils::build_string_prefix(key);
    
    if (obj.size() <= constants::word) {
        auto it = std::ranges::find_if(
			obj, 
			[this, key, target_prefix](const JsonMember& member) {
				if ((member.key_index_ >> constants::dword) != target_prefix) {
					return false;
				}
				return storage_->string(member.key_index_ & 0xFFFFFFFFULL) == key;
			}
		);
        if (it != obj.end()) {
            return fromInternal(storage_, it->value_);
        }
    } else {
        auto it = std::lower_bound(
			obj.begin(), 
			obj.end(), 
			key, 
			[this, target_prefix](const JsonMember& member, std::string_view k) {
				const unsigned long long member_prefix = member.key_index_ >> constants::dword;
				if (member_prefix != target_prefix) {
					return member_prefix < target_prefix;
				}
				return storage_->string(member.key_index_ & 0xFFFFFFFFULL) < k;
			}
		);
        if (
			it != obj.end() && 
			(it->key_index_ >> constants::dword) == target_prefix && 
			storage_->string(it->key_index_ & 0xFFFFFFFFULL) == key
		) {
            return fromInternal(storage_, it->value_);
        }
    }

    return std::unexpected{core::JsonError::InvalidValue};
}

// Fluent operator[] (Optional Chaining safe)
Value Value::operator[](unsigned long long index) const noexcept {
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