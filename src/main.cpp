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
#include <iostream>

int
    main() {
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
        std::cerr << "Parse error: " << static_cast<int>(result.error()) << '\n';
        return 1;
    }

    const auto& doc = *result;
    const auto root = doc.root();

    std::cout << "--- FLUENT API SHOWCASE ---\n";

    // Mengambil nilai primitif langsung dengan get_XXX()
    std::cout << "name    : " << root["name"].get_string("unknown") << '\n';
    std::cout << "active  : " << (root["active"].get_bool() ? "true" : "false") << '\n';
    std::cout << "age     : " << root["age"].get_integer() << '\n';
    std::cout << "pi      : " << root["pi"].get_double() << '\n';

    // Optional Chaining: langsung akses nested object tanpa if-checker bertumpuk
    std::cout << "pos.x   : " << root["position"]["x"].get_double() << '\n';
    std::cout << "pos.y   : " << root["position"]["y"].get_double() << '\n';

    // Mengecek Null
    if (root["data"].is_null()) {
        std::cout << "data    : is explicitly null\n";
    }

    // Penanganan key yang tidak ada secara aman (tidak akan crash/throw exception)
    std::cout << "missing : " << root["invalid_key"]["deep_key"].get_string("default_string")
              << '\n';

    // Iterasi Array (menggunakan size() secara aman)
    auto colors = root["colors"];
    std::cout << "colors  : " << colors.size() << " item(s)\n";
    for (unsigned long long i = 0; i < colors.size(); ++i) {
        std::cout << "  [" << i << "]   : " << colors[i].get_string() << '\n';
    }

    return 0;
}