/**
 * @file fs_util.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Filesystem utilities for benchmark and testing.
 * @version 1.0.0
 * @date 2026-06-21
 * 
 * @copyright Copyright (c) 2026
 */

#pragma once

#include <expected>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <system_error>

namespace zuu::tests::utils {

[[nodiscard]] inline std::string get_sample_path(std::string_view filename) {
    std::filesystem::path cwd = std::filesystem::current_path();

    for (int i = 0; i < 4; ++i) {
        std::filesystem::path target = cwd / "samples" / filename;
        if (std::filesystem::exists(target)) {
            return target.string();
        }
        cwd = cwd.parent_path();
    }
    return "";
}

[[nodiscard]] inline std::expected<std::string, std::errc>
load_sample(std::string_view filename) noexcept {
    std::string filepath = get_sample_path(filename);
    if (filepath.empty()) {
        return std::unexpected{std::errc::no_such_file_or_directory};
    }

    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        return std::unexpected{std::errc::no_such_file_or_directory};
    }

    file.seekg(0, std::ios::end);
    const auto size = file.tellg();
    if (size < 0) {
        return std::unexpected{std::errc::io_error};
    }

    std::string result(static_cast<std::size_t>(size), '\0');
    file.seekg(0, std::ios::beg);

    file.read(result.data(), static_cast<std::streamsize>(result.size()));
    if (!file && !file.eof()) {
        return std::unexpected{std::errc::io_error};
    }

    result.resize(static_cast<std::size_t>(file.gcount()));
    return result;
}

} // namespace zuu::tests::utils