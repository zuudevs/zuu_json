/**
 * @file example_basic.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Contoh penggunaan dasar zuu_json (Parsing & Fluent API)
 * @version 1.0.0
 * @date 2026-06-16
 *
 * @copyright Copyright (c) 2026
 */

#include "zuu_json/json.hpp"
#include <iostream>

int main() {
    std::string_view json_input = R"({
        "nama": "zuu",
        "umur": 20,
        "is_student": true
    })";

    // 1. Parsing string JSON
    auto doc = zuu::Json::parse(json_input);
    if (!doc) {
        std::cerr << "Gagal parsing JSON!\n";
        return 1;
    }

    // 2. Ambil root (akar) dari dokumen
    auto root = doc->root();

    // 3. Mengambil data dengan Fluent API (Aman)
    std::cout << "Nama   : " << root["nama"].get_string("Tidak diketahui") << '\n';
    std::cout << "Umur   : " << root["umur"].get_integer(0) << '\n';
    std::cout << "Pelajar: " << root["is_student"].get_bool(false) << '\n';

    // Jika key tidak ada, nilai default akan dikembalikan
    std::cout << "Hobi   : " << root["hobi"].get_string("Belum ada hobi") << '\n';
}