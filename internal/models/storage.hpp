/**
 * @file storage.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-06
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "models/json_member.hpp"
#include "models/token.hpp"
#include <cstdint>
#include <memory>
#include <span>
#include <string_view>
#include "traits/meta_trait.hpp"

namespace zuu::models {

class Storage;

} // namespace zuu::models

namespace zuu::traits {

template <>
struct MetaTrait<models::Storage> {
	uint32_t offset{};
    uint32_t size{};
    bool is_sorted{};
};

} // namespace zuu::traits

namespace zuu::models {

class Storage {
  public:
    using Type = JsonValue::Type;
	using Hint = traits::HintTrait<Token>;
    using JsonArray = std::span<const JsonValue>;
    using JsonObject = std::span<const JsonMember>;

    Storage() noexcept = default;
    Storage(const Storage&) = delete;
    Storage(Storage&&) noexcept = default;
    Storage& operator=(const Storage&) = delete;
    Storage& operator=(Storage&&) noexcept = default;
    ~Storage() noexcept = default;

    Storage(Hint hint) noexcept;

    [[nodiscard]] bool hasRoot() const noexcept;
    void setRoot(JsonValue value) noexcept;
    [[nodiscard]] const JsonValue& root() const noexcept;

    [[nodiscard]] uint64_t commitString(std::string_view value) noexcept;
    [[nodiscard]] char* allocateStringBuffer(uint64_t length) noexcept;

    [[nodiscard]] uint64_t getArrayOffset() const noexcept;
    void pushArrayElement(const JsonValue& val) noexcept;
    [[nodiscard]] uint64_t sealArray(uint64_t start_offset) noexcept;

    [[nodiscard]] uint64_t getObjectOffset() const noexcept;
    void pushObjectMember(const JsonMember& member) noexcept;
    [[nodiscard]] uint64_t sealObject(uint64_t start_offset) noexcept;
	[[nodiscard]] bool isObjectSorted(uint64_t index) const noexcept;
	void sortAllObjects() noexcept;

    [[nodiscard]] JsonArray array(uint64_t index) const noexcept;
    [[nodiscard]] JsonObject object(uint64_t index) const noexcept;
    [[nodiscard]] std::string_view string(uint64_t index) const noexcept;
	[[nodiscard]] std::string_view resolveKey(const JsonMember& member) const noexcept;

  private:
    std::unique_ptr<std::byte[]> arena_;
    std::string_view* strings_{nullptr};
    std::pair<uint32_t, uint32_t>* arrays_{nullptr};
	traits::MetaTrait<Storage>* objects_{nullptr};
	JsonValue* array_elements_{nullptr};
    JsonMember* object_elements_{nullptr};
    char* string_buffer_{nullptr};
    JsonValue root_{};

	uint32_t strings_size_{0};
    uint32_t array_elements_size_{0};
	uint32_t arrays_size_{0};
    uint32_t object_elements_size_{0};
    uint32_t objects_size_{0};
    uint32_t string_buffer_size_{0};

    bool root_set_{false};
};

} // namespace zuu::models