/**
 * @file main.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-16
 *
 * @copyright Copyright (c) 2026
 */

#include "zuu_json/json.hpp"
#include <print>

int main() {
    constexpr std::string_view json_input = R"({
        "name"    : "zuu",
        "active"  : true,
        "age"     : 21,
        "pi"      : 3.14,
        "data"    : null,
        "position": { "x": 20.5, "y": 10.5 },
        "colors"  : ["white", "blue"]
    })";

    auto result = zuu::Json::parse(json_input);
    if (!result) {
        std::println("Parse error: {}", static_cast<int>(result.error()));
        return 1;
    }

    const auto& doc = *result;
    const auto root = doc.root();

    std::println("--- FLUENT API SHOWCASE ---");

    // Mengambil nilai primitif langsung dengan get_XXX()
    std::println("name    : {}", root["name"].get_string("unknown"));
    std::println("active  : {}", root["active"].get_bool());
    std::println("age     : {}", root["age"].get_integer());
    std::println("pi      : {}", root["pi"].get_double());

    // Optional Chaining: langsung akses nested object tanpa if-checker bertumpuk
    std::println("pos.x   : {}", root["position"]["x"].get_double());
    std::println("pos.y   : {}", root["position"]["y"].get_double());

    // Mengecek Null
    if (root["data"].is_null()) {
        std::println("data    : is explicitly null");
    }

    // Penanganan key yang tidak ada secara aman (tidak akan crash/throw exception)
    std::println("missing : {}", root["invalid_key"]["deep_key"].get_string("default_string"));

    // Iterasi Array (menggunakan size() secara aman)
    auto colors = root["colors"];
    std::println("colors  : {} item(s)", colors.size());
    for (unsigned long long i = 0; i < colors.size(); ++i) {
        std::println("  [{}]   : {}", i, colors[i].get_string());
    }

    return 0;
}