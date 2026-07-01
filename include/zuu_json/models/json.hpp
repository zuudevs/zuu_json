/**
 * @file json.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-05
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "policies.hpp"
#include "zuu_json/models/value.hpp"
#include <expected>
#include <memory>

namespace zuu::allocators {

class Storage;

} // namespace zuu::allocators

namespace zuu::models {

/**
 * @brief A fully-parsed JSON document.  Move-only.
 *
 * Usage:
 * @code
 * auto doc = Json::parse(R"({"name":"zuu","age":21})");
 * if (!doc) { handle(doc.error()); }
 * * doc->sort(); // Call this manually if you need fast (O(log N)) lookups later
 * * auto name = (*doc)["name"]->as_string().value();  // "zuu"
 * auto age  = (*doc)["age"]->as_integer().value();  // 21
 * @endcode
 */
class Json {
  public:
    using Error = core::JsonError;
    template <typename T>
    using Result = std::expected<T, Error>;

    Json(Json&&) noexcept;
    Json&
        operator=(Json&&) noexcept;
    ~Json() noexcept;

    Json(const Json&) = delete;
    Json&
        operator=(const Json&) = delete;

    [[nodiscard]] static Result<Json>
        parse(std::string_view content, Policy policy = default_policy) noexcept;
    void
        sort() noexcept;
    [[nodiscard]] Value
        root() const noexcept;
    [[nodiscard]] Result<Value>
        operator[](std::string_view key) const noexcept;
    [[nodiscard]] std::string
        dump(int indent = -1) const noexcept;

  private:
    explicit Json(std::unique_ptr<allocators::Storage> storage) noexcept;
    std::unique_ptr<allocators::Storage> storage_;
};

} // namespace zuu::models