/**
 * @file filesystem.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-10
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include <expected>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <string_view>
#include <system_error>

namespace zuu::utils {

class FileSystem {
  public:
    template <typename T>
    using Expected = std::expected<T, std::errc>;
    using Unexpected = std::unexpected<std::errc>;

    [[nodiscard]] static Expected<std::string> ReadFile(std::string_view filepath) noexcept {
        std::ifstream file(std::string(filepath), std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            return Unexpected{std::errc::no_such_file_or_directory};
        }

        file.seekg(0, std::ios::end);
        const auto size = file.tellg();
        if (size < 0) {
            return Unexpected{std::errc::io_error};
        }

        std::string result(static_cast<std::size_t>(size), '\0');
        file.seekg(0, std::ios::beg);

        file.read(result.data(), static_cast<std::streamsize>(result.size()));
        if (!file && !file.eof()) {
            return Unexpected{std::errc::io_error};
        }

        result.resize(static_cast<std::size_t>(file.gcount()));
        return result;
    }

    static Expected<void> WriteFile(std::string_view filepath,
                                    std::span<const char> content,
                                    bool append = false) noexcept {
        std::ofstream file(std::string(filepath),
                           std::ios::out | std::ios::binary |
                               (append ? std::ios::app : std::ios::trunc));

        if (!file.is_open()) {
            return Unexpected{std::errc::permission_denied};
        }

        file.write(content.data(), static_cast<std::streamsize>(content.size()));
        if (!file) {
            return Unexpected{std::errc::io_error};
        }

        return {};
    }
};

} // namespace zuu::utils