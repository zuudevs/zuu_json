/**
 * @file storage.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Dynamic Chunked Arena implementation
 * @version 1.5.0
 * @date 2026-06-29
 *
 * @copyright Copyright (c) 2026
 */

#include "models/storage.hpp"
#include "constants/general.hpp"
#include <algorithm>
#include <utility>
#include <cstring>

namespace zuu::models {

Storage::Storage() noexcept {
    head_ = new Chunk(default_chunk_size_);
    tail_ = head_;
    
    strings_.reserve(1024);
    arrays_.reserve(512);
    objects_.reserve(512);
}

Storage::~Storage() noexcept {
    Chunk* curr = head_;
    while (curr) {
        Chunk* next = curr->next;
        delete curr;
        curr = next;
    }
}

Storage::Storage(Storage&& other) noexcept
    : head_(std::exchange(other.head_, nullptr)),
      tail_(std::exchange(other.tail_, nullptr)),
      default_chunk_size_(other.default_chunk_size_),
      strings_(std::move(other.strings_)),
      arrays_(std::move(other.arrays_)),
      objects_(std::move(other.objects_)),
      total_array_elements_(other.total_array_elements_),
      total_object_elements_(other.total_object_elements_),
      root_(other.root_),
      root_set_(other.root_set_) {}

Storage& Storage::operator=(Storage&& other) noexcept {
    if (this != &other) {
        Chunk* curr = head_;
        while (curr) {
            Chunk* next = curr->next;
            delete curr;
            curr = next;
        }
        head_ = std::exchange(other.head_, nullptr);
        tail_ = std::exchange(other.tail_, nullptr);
        default_chunk_size_ = other.default_chunk_size_;
        strings_ = std::move(other.strings_);
        arrays_ = std::move(other.arrays_);
        objects_ = std::move(other.objects_);
        total_array_elements_ = other.total_array_elements_;
        total_object_elements_ = other.total_object_elements_;
        root_ = other.root_;
        root_set_ = other.root_set_;
    }
    return *this;
}

void* Storage::allocate(uint32_t size, uint32_t alignment) noexcept {
    uint32_t align_offset = (alignment - (reinterpret_cast<uintptr_t>(tail_->data.get() + tail_->used) % alignment)) % alignment;
    
    if (tail_->used + align_offset + size > tail_->capacity) {
        // Alokasikan chunk baru (Linked List)
        uint32_t new_cap = std::max(default_chunk_size_, size + align_offset);
        tail_->next = new Chunk(new_cap);
        tail_ = tail_->next;
        align_offset = (alignment - (reinterpret_cast<uintptr_t>(tail_->data.get()) % alignment)) % alignment;
    }
    
    tail_->used += align_offset;
    void* ptr = tail_->data.get() + tail_->used;
    tail_->used += size;
    return ptr;
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

uint64_t Storage::commitString(std::string_view value) noexcept {
    strings_.push_back(value);
    return strings_.size() - 1;
}

char* Storage::allocateStringBuffer(uint64_t length) noexcept {
    return static_cast<char*>(allocate(length, 1));
}

uint64_t Storage::sealArray(const JsonValue* elements, uint32_t count) noexcept {
    total_array_elements_ += count;
    if (count == 0) {
        arrays_.push_back({nullptr, 0});
        return arrays_.size() - 1;
    }
    
    auto* ptr = static_cast<JsonValue*>(allocate(count * sizeof(JsonValue), alignof(JsonValue)));
    
    std::memcpy(ptr, elements, count * sizeof(JsonValue));
    
    arrays_.push_back({ptr, count});
    return arrays_.size() - 1;
}

uint64_t Storage::sealObject(const JsonMember* members, uint32_t count) noexcept {
    total_object_elements_ += count;
    if (count == 0) {
        objects_.push_back({nullptr, 0, false});
        return objects_.size() - 1;
    }
    
    auto ptr = static_cast<JsonMember*>(allocate(count * sizeof(JsonMember), alignof(JsonMember)));
    
    std::memcpy(ptr, members, count * sizeof(JsonMember));
    
    objects_.push_back({ptr, count, false});
    return objects_.size() - 1;
}

bool Storage::isObjectSorted(uint64_t index) const noexcept {
    return objects_[index].is_sorted;
}

void Storage::sortAllObjects() noexcept {
    for (auto& meta : objects_) {
        if (!meta.is_sorted && meta.size > 1) {
            auto ptr = const_cast<JsonMember*>(meta.ptr);
            std::ranges::sort(
                ptr, ptr + meta.size,
                [](std::string_view a, std::string_view b) {
                    if (a.size() != b.size()) {
                        return a.size() < b.size();
                    }
                    return a < b;
                },
                [this](const JsonMember& member) { 
                    return resolveKey(member); 
                }
            );
            meta.is_sorted = true;
        }
    }
}

Storage::JsonArray Storage::array(uint64_t index) const noexcept {
    const auto& meta = arrays_[index];
    return {meta.first, meta.second};
}

Storage::JsonObject Storage::object(uint64_t index) const noexcept {
    const auto& meta = objects_[index];
    return {meta.ptr, meta.size};
}

std::string_view Storage::string(uint64_t index) const noexcept {
    return strings_[index];
}

std::string_view Storage::resolveKey(const JsonMember& member) const noexcept {
    if ((member.key_.sso_.tag_ & constants::sso_tag) != 0) {
        const uint8_t len = member.key_.sso_.tag_ & ~constants::sso_tag;
        return {member.key_.sso_.chars_, len};
    }
    return strings_[member.key_.ref_.index_];
}

} // namespace zuu::models