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

#include "zuu_json/core/error.hpp"
#include "zuu_json/models/value.hpp"
#include <expected>
#include <memory>

namespace zuu::models {

class Storage;

/**
 * @brief A fully-parsed JSON document.  Move-only.
 *
 * Usage:
 * @code
 *   auto doc = Json::parse(R"({"name":"zuu","age":21})");
 *   if (!doc) { handle(doc.error()); }
 *
 *   auto name = (*doc)["name"]->as_string().value();  // "zuu"
 *   auto age  = (*doc)["age"]->as_integer().value();  // 21
 * @endcode
 */
class Json {
  public:
    Json(Json&&) noexcept;
    Json& operator=(Json&&) noexcept;
    ~Json() noexcept;

    Json(const Json&) = delete;
    Json& operator=(const Json&) = delete;

    [[nodiscard]] static std::expected<Json, core::JsonError>
    parse(std::string_view content) noexcept;

    [[nodiscard]] Value root() const noexcept;

    [[nodiscard]] std::expected<Value, core::JsonError>
    operator[](std::string_view key) const noexcept;

  private:
    explicit Json(std::unique_ptr<Storage> storage) noexcept;

    std::unique_ptr<Storage> storage_;
};

} // namespace zuu::models