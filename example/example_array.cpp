/**
 * @file example_array.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-16
 *
 * @copyright Copyright (c) 2026
 */

#include <iostream>
#include "zuu_json/json.hpp"

int main() {
    std::string_view json_input = R"({
        "bahasa_favorit": ["C", "C++", "Python", "Rust"]
    })";

    auto doc = zuu::Json::parse(json_input).value();
    auto root = doc.root();

    // Ambil array
    auto bahasa = root["bahasa_favorit"];

    // Pastikan itu benar-benar array
    if (bahasa.is_array()) {
        std::cout << "zuu punya "<< bahasa.size() << " bahasa pemrograman favorit:\n";

        // Iterasi array secara aman
        for (unsigned long long i = 0; i < bahasa.size(); ++i) {
            std::cout << i + 1 << ". " << bahasa[i].get_string() << '\n';
        }
    }
}