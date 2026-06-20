/**
 * @file example_sort.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Contoh penggunaan manual sort untuk pencarian O(log N)
 * @version 1.0.1
 * @date 2026-06-20
 *
 * @copyright Copyright (c) 2026
 */

#include "zuu_json/json.hpp"
#include <iostream>

int main() {
    std::string_view json_input = R"({
        "zulu": 1,
        "alpha": 2,
        "delta": 3,
        "bravo": 4
    })";

    // 1. Parsing super cepat (TANPA SORT)
    auto doc = zuu::Json::parse(json_input);
    if (!doc) {
        std::cerr << "Gagal parsing JSON!\n";
        return 1;
    }

    // 2. Baca data sesekali -> Menggunakan Linear Search O(N) secara otomatis
    auto root = doc->root();
    std::cout << "Sebelum sort, nilai 'delta': " << root["delta"].get_integer() << '\n';

    // 3. Kita sadar kita akan baca data jutaan kali, mari kita sort secara manual!
    doc->sort();
    std::cout << "Dokumen berhasil disortir secara lazy!\n";

    // 4. Baca data lagi -> Karena sudah di-sort, ia akan otomatis menggunakan Binary Search O(log N)
    std::cout << "Setelah sort, nilai 'alpha': " << root["alpha"].get_integer() << '\n';

    return 0;
}