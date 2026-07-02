/**
 * @file serializer.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Zero-allocation overhead JSON serialization engine
 * @version 1.0.0
 * @date 2026-06-21
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "allocators/storage.hpp"
#include <string>

namespace zuu::serializer {

class Serializer {
  public:
    /**
     * @brief Menjalankan traversal mendalam (DFS) dan merangkai string JSON.
     * @param storage Pointer ke memori arena.
     * @param root Node/Value awal yang ingin diserialisasi.
     * @param indent Tingkat indentasi spasi (-1 untuk minified / tanpa spasi).
     * @return std::string JSON hasil akhir.
     */
    [[nodiscard]] static std::string
        dump(const allocators::Storage* storage,
             const models::JsonValue& root,
             int indent) noexcept;

  private:
    std::string out_;
    int indent_;
    int current_indent_{0};
    const allocators::Storage* storage_;

    Serializer(const allocators::Storage* storage, int indent) noexcept;

    void
        serializeValue(const models::JsonValue& value) noexcept;
    void
        serializeObject(const models::JsonValue& value) noexcept;
    void
        serializeArray(const models::JsonValue& value) noexcept;
    void
        serializeString(std::string_view str) noexcept;
    void
        serializeInteger(long long val) noexcept;
    void
        serializeDouble(double val) noexcept;
    void
        writeIndent() noexcept;
};

} // namespace zuu::serializer