/**
 * @file storage.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-06
 *
 * @copyright Copyright (c) 2026
 */

#include <algorithm>
#include "constants/general.hpp"
#include "models/storage.hpp"

namespace zuu::models {

[[nodiscard]] inline constexpr std::size_t align_to_cache_line(std::size_t size) noexcept {
    return (size + constants::cache_line_size - 1) & ~(constants::cache_line_size - 1);
}

Storage::Storage(Hint hint) noexcept {
    const unsigned long long max_strings = hint.string_count_;
    const unsigned long long max_arrays = hint.array_count_;
    const unsigned long long max_objects = hint.object_count_;
    const unsigned long long max_elements =
        hint.comma_count_ + hint.array_count_ + hint.object_count_;

    const unsigned long long strings_bytes    = align_to_cache_line(max_strings * sizeof(std::string_view));
    const unsigned long long array_elem_bytes = align_to_cache_line(max_elements * sizeof(JsonValue));
    const unsigned long long arrays_bytes     = align_to_cache_line(max_arrays * sizeof(std::pair<unsigned, unsigned>));
    const unsigned long long obj_elem_bytes   = align_to_cache_line(max_elements * sizeof(JsonMember));
    const unsigned long long objects_bytes    = align_to_cache_line(max_objects * sizeof(std::pair<unsigned, unsigned>));
    const unsigned long long str_buf_bytes    = align_to_cache_line(hint.string_escape_bytes_);

    const unsigned long long total_bytes = strings_bytes + array_elem_bytes + arrays_bytes +
                                           obj_elem_bytes + objects_bytes + str_buf_bytes;

    if (total_bytes > 0) {
        arena_ = std::make_unique_for_overwrite<std::byte[]>(total_bytes);
        auto raw_address = reinterpret_cast<std::uintptr_t>(arena_.get());
        auto aligned_address = 
		(raw_address + constants::cache_line_size - 1) & ~(constants::cache_line_size - 1);
        auto ptr = reinterpret_cast<std::byte*>(aligned_address);
        strings_ = reinterpret_cast<std::string_view*>(ptr);
        ptr += strings_bytes;

        array_elements_ = reinterpret_cast<JsonValue*>(ptr);
        ptr += array_elem_bytes;

        arrays_ = reinterpret_cast<std::pair<unsigned, unsigned>*>(ptr);
        ptr += arrays_bytes;

        object_elements_ = reinterpret_cast<JsonMember*>(ptr);
        ptr += obj_elem_bytes;

        objects_ = reinterpret_cast<std::pair<unsigned, unsigned>*>(ptr);
        ptr += objects_bytes;

        string_buffer_ = reinterpret_cast<char*>(ptr);
    }
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

// ── BUMP ALLOCATOR OPERATIONS ───────────────────────────────────────────────

unsigned long long Storage::commitString(std::string_view value) noexcept {
    strings_[strings_size_] = value;
    return strings_size_++;
}

char* Storage::allocateStringBuffer(unsigned long long length) noexcept {
    char* allocated = string_buffer_ + string_buffer_size_;
    string_buffer_size_ += static_cast<unsigned>(length);
    return allocated;
}

unsigned long long Storage::getArrayOffset() const noexcept {
    return array_elements_size_;
}

void Storage::pushArrayElement(const JsonValue& val) noexcept {
    array_elements_[array_elements_size_++] = val;
}

unsigned long long Storage::sealArray(unsigned long long start_offset) noexcept {
    const auto size = static_cast<unsigned>(array_elements_size_ - start_offset);
    arrays_[arrays_size_] = {static_cast<unsigned>(start_offset), size};
    return arrays_size_++;
}

unsigned long long Storage::getObjectOffset() const noexcept {
    return object_elements_size_;
}

void Storage::pushObjectMember(const JsonMember& member) noexcept {
    object_elements_[object_elements_size_++] = member;
}

unsigned long long Storage::sealObject(unsigned long long start_offset) noexcept {
    const auto size = static_cast<unsigned>(object_elements_size_ - start_offset);
    objects_[objects_size_] = {static_cast<unsigned>(start_offset), size};
    return objects_size_++;
}

// ─────────────────────────────────────────────────────────────────────────────

Storage::JsonArray Storage::array(unsigned long long index) const noexcept {
    const auto& [offset, size] = arrays_[index];
    return {array_elements_ + offset, size};
}

Storage::JsonObject Storage::object(unsigned long long index) const noexcept {
    const auto& [offset, size] = objects_[index];
    return {object_elements_ + offset, size};
}

std::string_view Storage::string(unsigned long long index) const noexcept {
    return strings_[index];
}

} // namespace zuu::models