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
#include <cstdint>
#include "models/storage.hpp"

namespace zuu::models {

[[nodiscard]] inline constexpr std::size_t align_to_cache_line(std::size_t size) noexcept {
    return (size + constants::cache_line_size - 1) & ~(constants::cache_line_size - 1);
}

Storage::Storage(Hint hint) noexcept {
    const uint64_t max_strings = hint.string_count_;
    const uint64_t max_arrays = hint.array_count_;
    const uint64_t max_objects = hint.object_count_;
    const uint64_t max_elements = 
	hint.comma_count_ + 
	hint.array_count_ + 
	hint.object_count_;

    const uint64_t strings_bytes       = align_to_cache_line(max_strings * sizeof(std::string_view));
    const uint64_t array_elem_bytes    = align_to_cache_line(max_elements * sizeof(JsonValue));
    const uint64_t arrays_bytes        = align_to_cache_line(max_arrays * sizeof(std::pair<uint32_t, uint32_t>));
    const uint64_t object_elem_bytes   = align_to_cache_line(max_elements * sizeof(JsonMember));
    const uint64_t objects_bytes       = align_to_cache_line(max_objects * sizeof(traits::MetaTrait<Storage>));
    const uint64_t str_buf_bytes       = align_to_cache_line(hint.string_escape_bytes_);

    const uint64_t total_bytes = 
        strings_bytes + array_elem_bytes + 
        arrays_bytes + object_elem_bytes + 
        objects_bytes + str_buf_bytes;

    if (total_bytes > 0) {
		arena_ = std::make_unique_for_overwrite<std::byte[]>(total_bytes + constants::cache_line_size - 1);
		auto raw_address = reinterpret_cast<std::uintptr_t>(arena_.get());
		auto aligned_address = 
		(raw_address + constants::cache_line_size - 1) & 
		~(constants::cache_line_size - 1);

        auto ptr = reinterpret_cast<std::byte*>(aligned_address);
        strings_ = reinterpret_cast<std::string_view*>(ptr);
        ptr += strings_bytes;

        array_elements_ = reinterpret_cast<JsonValue*>(ptr);
        ptr += array_elem_bytes;

        arrays_ = reinterpret_cast<std::pair<uint32_t, uint32_t>*>(ptr);
        ptr += arrays_bytes;

        object_elements_ = reinterpret_cast<JsonMember*>(ptr);
        ptr += object_elem_bytes;

        objects_ = reinterpret_cast<traits::MetaTrait<Storage>*>(ptr);
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

uint64_t Storage::commitString(std::string_view value) noexcept {
    strings_[strings_size_] = value;
    return strings_size_++;
}

char* Storage::allocateStringBuffer(uint64_t length) noexcept {
    char* allocated = string_buffer_ + string_buffer_size_;
    string_buffer_size_ += static_cast<uint32_t>(length);
    return allocated;
}

uint64_t Storage::getArrayOffset() const noexcept {
    return array_elements_size_;
}

void Storage::pushArrayElement(const JsonValue& val) noexcept {
    array_elements_[array_elements_size_++] = val;
}

uint64_t Storage::sealArray(uint64_t start_offset) noexcept {
    const auto size = static_cast<uint32_t>(array_elements_size_ - start_offset);
    arrays_[arrays_size_] = {
		static_cast<uint32_t>(start_offset), 
		size
	};
    return arrays_size_++;
}

uint64_t Storage::getObjectOffset() const noexcept {
    return object_elements_size_;
}

void Storage::pushObjectMember(const JsonMember& member) noexcept {
    object_elements_[object_elements_size_++] = member;
}

bool Storage::isObjectSorted(uint64_t index) const noexcept {
    return objects_[index].is_sorted;
}

uint64_t Storage::sealObject(uint64_t start_offset) noexcept {
    const auto size = static_cast<uint32_t>(object_elements_size_ - start_offset);
    objects_[objects_size_] = {
		.offset = static_cast<uint32_t>(start_offset), 
		.size = size,
		.is_sorted = false
	};
    return objects_size_++;
}

void Storage::sortAllObjects() noexcept {
    for (uint32_t i = 0; i < objects_size_; ++i) {
        if (!objects_[i].is_sorted && objects_[i].size > 1) {
            std::ranges::sort(
                object_elements_ + objects_[i].offset, 
                object_elements_ + objects_[i].offset + objects_[i].size,
                [this](const JsonMember& a, const JsonMember& b) {
                    return resolveKey(a) < resolveKey(b);
                }
            );
            objects_[i].is_sorted = true;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────

Storage::JsonArray Storage::array(uint64_t index) const noexcept {
    const auto& [offset, size] = arrays_[index];
    return {
		array_elements_ + offset, 
		size
	};
}

Storage::JsonObject Storage::object(uint64_t index) const noexcept {
    const auto& meta = objects_[index];
    return {
		object_elements_ + meta.offset, 
		meta.size
	};
}

std::string_view Storage::string(uint64_t index) const noexcept {
    return strings_[index];
}

std::string_view Storage::resolveKey(const JsonMember& member) const noexcept {
    return strings_[member.key_.index_];
}

} // namespace zuu::models