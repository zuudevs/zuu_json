/**
 * @file example_parsing_error.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-16
 *
 * @copyright Copyright (c) 2026
 */

#include "zuu_json/json.hpp"
#include "zuu_json/utils/error_translator.hpp"
#include <print>

int main() {
    // Sengaja membuat JSON yang salah (kurang kutip pada key)
    std::string_view bad_json = R"({
        nama: "zuu",
        "umur": 20
    })";

    auto result = zuu::Json::parse(bad_json);

    if (!result) {
        // TranslateError akan mengubah kode error (enum) menjadi teks yang bisa dibaca
        auto pesan_error = zuu::utils::TranslateError(result.error());
        std::println("Ops! Parsing gagal.");
        std::println("Alasan: {}", pesan_error);
        // Output yang diharapkan: "Unquoted key"
    }
}