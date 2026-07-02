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
#include "allocators/storage.hpp"
#include "models/views/array.hpp"
#include "models/views/object.hpp"
#include "serializer/serializer.hpp"
#include "utils/compiler.hpp"
#include <algorithm>

namespace zuu::models {

Value::Value(const allocators::Storage* storage, JsonValue value) noexcept
    : storage_(storage)
    , value_(value) {}

Value
    Value::fromInternal(const allocators::Storage* storage, const JsonValue& v) noexcept {
    return Value(storage, v);
}

Value
    Value::createNull(const allocators::Storage* storage) noexcept {
    return Value(storage, JsonValue::Null());
}

Value::Type
    Value::type() const noexcept {
    return value_.get_type();
}

bool
    Value::is_null() const noexcept {
    return value_.get_type() == Type::Null;
}

bool
    Value::is_bool() const noexcept {
    return value_.get_type() == Type::Boolean;
}

bool
    Value::is_integer() const noexcept {
    return value_.get_type() == Type::Integer;
}

bool
    Value::is_double() const noexcept {
    return value_.get_type() == Type::Double;
}

bool
    Value::is_string() const noexcept {
    return value_.get_type() == Type::String;
}

bool
    Value::is_array() const noexcept {
    return value_.get_type() == Type::Array;
}

bool
    Value::is_object() const noexcept {
    return value_.get_type() == Type::Object;
}

Value::Result<bool>
    Value::as_bool() const noexcept {
    if (value_.get_type() != Type::Boolean) {
        return std::unexpected{core::JsonError::InvalidType};
    }
    return value_.as_bool();
}

Value::Result<long long>
    Value::as_integer() const noexcept {
    if (value_.get_type() != Type::Integer) {
        return std::unexpected{core::JsonError::InvalidType};
    }
    return value_.as_integer();
}

Value::Result<long double>
    Value::as_double() const noexcept {
    if (value_.get_type() == Type::Integer) {
        return static_cast<long double>(value_.as_integer());
    }
    if (value_.get_type() != Type::Double) {
        return std::unexpected{core::JsonError::InvalidType};
    }
    return value_.as_double();
}

Value::Result<std::string_view>
    Value::as_string() const noexcept {
    if (value_.get_type() == Type::String) {
        return storage_->string(value_.as_index());
    }
    return std::unexpected{core::JsonError::InvalidType};
}

bool
    Value::get_bool(bool default_val) const noexcept {
    if (value_.get_type() != Type::Boolean) {
        return default_val;
    }
    return value_.as_bool();
}

long long
    Value::get_integer(long long default_val) const noexcept {
    if (value_.get_type() != Type::Integer) {
        return default_val;
    }
    return value_.as_integer();
}

double
    Value::get_double(double default_val) const noexcept {
    if (value_.get_type() == Type::Integer) {
        return static_cast<double>(value_.as_integer());
    }
    if (value_.get_type() != Type::Double) {
        return default_val;
    }
    return static_cast<double>(value_.as_double());
}

std::string_view
    Value::get_string(std::string_view default_val) const noexcept {
    if (value_.get_type() == Type::String) {
        return storage_->string(value_.as_index());
    }
    return default_val;
}

unsigned long long
    Value::size() const noexcept {
    if (value_.get_type() == Type::Array) {
        return storage_->array(value_.as_index()).size();
    }
    if (value_.get_type() == Type::Object) {
        return storage_->object(value_.as_index()).size();
    }
    return 0;
}

ZUU_HOT
    ZUU_ALIGN(64) bool Value::contains(std::string_view key) const noexcept {
    if (value_.get_type() != Type::Object) {
        return false;
    }

    const auto obj_index = value_.as_index();
    const auto obj = storage_->object(obj_index);
    const bool is_sorted = storage_->isObjectSorted(obj_index);

    if (is_sorted) {
        auto it = std::lower_bound(obj.begin(),
                                   obj.end(),
                                   key,
                                   [this](const JsonMember& member, std::string_view search_key) {
                                       uint32_t mem_len =
                                           (member.key_.sso_.tag_ & constants::sso_tag)
                                               ? (member.key_.sso_.tag_ & ~constants::sso_tag)
                                               : member.key_.ref_.length_;

                                       if (mem_len != search_key.size()) {
                                           return mem_len < search_key.size();
                                       }
                                       return storage_->resolveKey(member) < search_key;
                                   });
        return (it != obj.end() && storage_->resolveKey(*it) == key);
    } else {
        auto it = std::ranges::find_if(obj, [this, key](const JsonMember& member) {
            auto m_key = storage_->resolveKey(member);
            return m_key.size() == key.size() && m_key == key;
        });
        return it != obj.end();
    }
}

Value::Result<views::Array>
    Value::as_array() const noexcept {
    if (value_.get_type() != Type::Array) {
        return std::unexpected{core::JsonError::IsNotArray};
    }
    return views::Array(storage_, storage_->array(value_.as_index()));
}

Value::Result<views::Object>
    Value::as_object() const noexcept {
    if (value_.get_type() != Type::Object) {
        return std::unexpected{core::JsonError::IsNotObject};
    }
    return views::Object(storage_, storage_->object(value_.as_index()));
}

views::Array
    Value::get_array() const noexcept {
    if (value_.get_type() != Type::Array) {
        return views::Array{storage_, {}};
    }
    return views::Array{storage_, storage_->array(value_.as_index())};
}

views::Object
    Value::get_object() const noexcept {
    if (value_.get_type() != Type::Object) {
        return views::Object{storage_, {}};
    }
    return views::Object{storage_, storage_->object(value_.as_index())};
}

std::string
    Value::dump(int indent) const noexcept {
    if (value_.get_type() == Type::Null && storage_ == nullptr) {
        return "null";
    }
    return serializer::Serializer::dump(storage_, value_, indent);
}

ZUU_HOT
    ZUU_ALIGN(64) Value::Result<Value> Value::at(unsigned long long index) const noexcept {
    if (value_.get_type() != Type::Array) {
        return std::unexpected{core::JsonError::IsNotArray};
    }

    const auto arr = storage_->array(value_.as_index());
    if (index >= arr.size()) {
        return std::unexpected{core::JsonError::InvalidValue};
    }

    return fromInternal(storage_, arr[index]);
}

ZUU_HOT
    ZUU_ALIGN(64) Value::Result<Value> Value::at(std::string_view key) const noexcept {
    if (value_.get_type() != Type::Object) {
        return std::unexpected{core::JsonError::IsNotObject};
    }

    const auto obj_index = value_.as_index();
    const auto obj = storage_->object(obj_index);
    const bool is_sorted = storage_->isObjectSorted(obj_index);

    if (is_sorted) {
        auto it = std::lower_bound(obj.begin(),
                                   obj.end(),
                                   key,
                                   [this](const JsonMember& member, std::string_view search_key) {
                                       uint32_t mem_len =
                                           (member.key_.sso_.tag_ & constants::sso_tag)
                                               ? (member.key_.sso_.tag_ & ~constants::sso_tag)
                                               : member.key_.ref_.length_;

                                       if (mem_len != search_key.size()) {
                                           return mem_len < search_key.size();
                                       }
                                       return storage_->resolveKey(member) < search_key;
                                   });
        if (it != obj.end() && storage_->resolveKey(*it) == key) {
            return fromInternal(storage_, it->value_);
        }
    } else {
        auto it = std::ranges::find_if(obj, [this, key](const JsonMember& member) {
            auto m_key = storage_->resolveKey(member);
            return m_key.size() == key.size() && m_key == key;
        });
        if (it != obj.end()) {
            return fromInternal(storage_, it->value_);
        }
    }
    return std::unexpected{core::JsonError::InvalidValue};
}

ZUU_HOT
    ZUU_ALIGN(64) Value Value::operator[](unsigned long long index) const noexcept {
    auto res = at(index);
    if (res) {
        return res.value();
    }
    return createNull(storage_);
}

ZUU_HOT
    ZUU_ALIGN(64) Value Value::operator[](std::string_view key) const noexcept {
    auto res = at(key);
    if (res) {
        return res.value();
    }
    return createNull(storage_);
}

} // namespace zuu::models