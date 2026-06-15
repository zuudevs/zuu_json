/**
 * @file storage.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-06
 *
 * @copyright Copyright (c) 2026
 */

#include "models/storage.hpp"

namespace zuu::models {

Storage::Storage(size_t strings_cap, size_t arrays_cap, size_t objects_cap) noexcept {
    strings_.reserve(strings_cap);
    arrays_.reserve(arrays_cap);
    objects_.reserve(objects_cap);
}

bool Storage::hasRoot() const noexcept {
    return root_set_;
}

void Storage::setRoot(JsonValue value) noexcept {
    root_ = value;
    root_set_ = true;
}

const JsonValue& Storage::root() const noexcept {
    return root_;
}

size_t Storage::addString(std::string_view value) noexcept {
    strings_.emplace_back(value);
    return strings_.size() - 1;
}

size_t Storage::addArray() noexcept {
    arrays_.emplace_back();
    return arrays_.size() - 1;
}

size_t Storage::addObject() noexcept {
    objects_.emplace_back();
    return objects_.size() - 1;
}

Storage::JsonArray& Storage::array(size_t index) noexcept {
    return arrays_[index];
}

const Storage::JsonArray& Storage::array(size_t index) const noexcept {
    return arrays_[index];
}

Storage::JsonObject& Storage::object(size_t index) noexcept {
    return objects_[index];
}

const Storage::JsonObject& Storage::object(size_t index) const noexcept {
    return objects_[index];
}

const std::string& Storage::string(size_t index) const noexcept {
    return strings_[index];
}

} // namespace zuu::models