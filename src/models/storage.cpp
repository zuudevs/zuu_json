/**
 * @file storage.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-06
 *
 * @copyright Copyright (c) 2026
 */

#include "models/storage.hpp"
#include <algorithm>

namespace zuu::models {

Storage::Storage(Hint hint) noexcept {
    const unsigned long long max_strings = hint.string_count_;
    const unsigned long long max_arrays = hint.array_count_;
    const unsigned long long max_objects = hint.object_count_;
    const unsigned long long max_elements =
        hint.comma_count_ + hint.array_count_ + hint.object_count_;

    const unsigned long long strings_bytes = max_strings * sizeof(std::string_view);
    const unsigned long long array_elem_bytes = max_elements * sizeof(JsonValue);
    const unsigned long long arrays_bytes = max_arrays * sizeof(std::pair<unsigned, unsigned>);
    const unsigned long long obj_elem_bytes = max_elements * sizeof(JsonMember);
    const unsigned long long objects_bytes = max_objects * sizeof(std::pair<unsigned, unsigned>);

    // Alokasi ruang untuk unescaped characters (beserta margin aman 10%)
    const unsigned long long str_buf_bytes = hint.string_escape_bytes_;

    const unsigned long long total_bytes = strings_bytes + array_elem_bytes + arrays_bytes +
                                           obj_elem_bytes + objects_bytes + str_buf_bytes;

    if (total_bytes > 0) {
        arena_ = std::make_unique<std::byte[]>(total_bytes);

        std::byte* ptr = arena_.get();

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

        // Ditempatkan paling akhir agar tidak merusak byte alignment dari tipe data lain di atasnya
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
    if (size > 1) {
        std::sort(object_elements_ + start_offset,
                  object_elements_ + object_elements_size_,
                  [this](const JsonMember& a, const JsonMember& b) {
                      return strings_[a.key_index_] < strings_[b.key_index_];
                  });
    }
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