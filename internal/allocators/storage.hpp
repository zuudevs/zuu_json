/**
 * @file storage.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Dynamic Chunked Arena for AST Storage
 * @version 1.5.0
 * @date 2026-06-29
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

#include "models/json_member.hpp"
#include "models/specs/storage.hpp"

namespace zuu::allocators {

class Storage {
  public:
    using Type = enums::JsonType;
    using JsonArray = std::span<const models::JsonValue>;
    using JsonObject = std::span<const models::JsonMember>;

    Storage() noexcept;
    Storage(Storage&& other) noexcept;
    Storage&
        operator=(Storage&& other) noexcept;
    ~Storage() noexcept;

    Storage(const Storage&) = delete;
    Storage&
        operator=(const Storage&) = delete;

    [[nodiscard]] bool
        hasRoot() const noexcept;
    void
        setRoot(models::JsonValue value) noexcept;
    [[nodiscard]] const models::JsonValue&
        root() const noexcept;

    [[nodiscard]] uint64_t
        commitString(std::string_view value) noexcept;
    [[nodiscard]] char*
        allocateStringBuffer(uint64_t length) noexcept;

    [[nodiscard]] uint64_t
        sealArray(const models::JsonValue* elements, uint32_t count) noexcept;
    [[nodiscard]] uint64_t
        sealObject(const models::JsonMember* members, uint32_t count) noexcept;

    [[nodiscard]] bool
        isObjectSorted(uint64_t index) const noexcept;
    void
        sortAllObjects() noexcept;

    [[nodiscard]] JsonArray
        array(uint64_t index) const noexcept;
    [[nodiscard]] JsonObject
        object(uint64_t index) const noexcept;
    [[nodiscard]] std::string_view
        string(uint64_t index) const noexcept;
    [[nodiscard]] std::string_view
        resolveKey(const models::JsonMember& member) const noexcept;

    // Untuk mengestimasi ukuran serialisasi
    [[nodiscard]] uint64_t
        getArrayElementsCount() const noexcept {
        return total_array_elements_;
    }
    [[nodiscard]] uint64_t
        getObjectElementsCount() const noexcept {
        return total_object_elements_;
    }

  private:
    struct Chunk {
        std::unique_ptr<std::byte[]> data;
        uint32_t capacity;
        uint32_t used;
        Chunk* next;
        Chunk(uint32_t cap)
            : capacity(cap)
            , used(0)
            , next(nullptr) {
            data = std::make_unique_for_overwrite<std::byte[]>(cap);
        }
    };

    Chunk* head_{nullptr};
    Chunk* tail_{nullptr};
    uint32_t default_chunk_size_{65536}; // 64KB per chunk

    [[nodiscard]] void*
        allocate(uint32_t size, uint32_t alignment) noexcept;

    std::vector<std::string_view> strings_;
    std::vector<std::pair<const models::JsonValue*, uint32_t>> arrays_;
    std::vector<models::specs::Storage> objects_;

    uint32_t total_array_elements_{0};
    uint32_t total_object_elements_{0};

    models::JsonValue root_{};
    bool root_set_{false};
};

} // namespace zuu::allocators