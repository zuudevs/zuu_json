/**
 * @file example_strict.cpp
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
    std::string_view json_input = R"({ "skor": 95 })";
    auto doc = zuu::Json::parse(json_input);
    auto root = doc->root();

    // Strict API mengembalikan Result<T> (std::expected)
    auto skor_result = root["skor"].as_integer();

    if (skor_result.has_value()) {
        std::println("Skor berhasil diambil: {}", skor_result.value());
    } else {
        auto err_msg = zuu::utils::TranslateError(skor_result.error());
        std::println("Gagal ambil skor: {}", err_msg);
    }

    // Mari kita coba ambil "skor" sebagai String (Pasti error karena tipenya Integer)
    auto salah_tipe = root["skor"].as_string();
    if (!salah_tipe) {
        std::println("Error tipe data: {}", zuu::utils::TranslateError(salah_tipe.error()));
    }
}