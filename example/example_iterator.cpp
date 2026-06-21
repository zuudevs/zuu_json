/**
 * @file example_iterator.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Contoh penggunaan Iterator standar C++20 dan Structured Bindings
 * @version 1.0.0
 * @date 2026-06-21
 *
 * @copyright Copyright (c) 2026
 */

#include "zuu_json/json.hpp"
#include <iostream>
#include <algorithm>
#include <ranges>

int main() {
    std::string_view json_input = R"({
        "metadata": "Server Log",
        "colors": ["red", "green", "blue"],
        "users": {
            "admin": 1,
            "guest": 0,
            "moderator": 2
        }
    })";

    auto doc = zuu::Json::parse(json_input).value();
    auto root = doc.root();

    std::cout << "--- Iterasi Array ---\n";
    // get_array() memastikan ini aman dari exception.
    // Jika "colors" tidak ada atau bukan array, loop ini langsung dilewati.
    for (const auto& color : root["colors"].get_array()) {
        std::cout << "- " << color.get_string() << '\n';
    }

    std::cout << "\n--- Iterasi Objek (Structured Bindings) ---\n";
    // get_object() mereturn MemberView yang kompatibel dengan C++17 Structured Bindings: auto [key, value]
    for (const auto& [key, val] : root["users"].get_object()) {
        std::cout << "User: " << key << " | Level: " << val.get_integer() << '\n';
    }

    std::cout << "\n--- Kompatibilitas Algoritma Standar (C++20 Ranges) ---\n";
    auto colors_view = root["colors"].get_array();
    
    // Karena iterator kita menggunakan random_access_iterator_tag dan spaceship operator,
    // ia kompatibel 100% dengan algoritma standar dari header <algorithm>.
    auto it = std::ranges::find_if(colors_view, [](const zuu::Value& v) {
        return v.get_string() == "green";
    });

    if (it != colors_view.end()) {
        std::cout << "Ditemukan warna: " << (*it).get_string() << " pada index " 
                  << std::distance(colors_view.begin(), it) << '\n';
    }

    return 0;
}