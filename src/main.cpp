/**
 * @file main.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-05
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

    const auto& json = *result;

    if (auto v = json["name"]) {
        if (auto s = v->as_string()) {
            std::println("name    : {}", *s);
        }
    }

    if (auto v = json["active"]) {
        if (auto b = v->as_bool()) {
            std::println("active  : {}", *b);
        }
    }

    if (auto v = json["age"]) {
        if (auto i = v->as_integer()) {
            std::println("age     : {}", *i);
        }
    }

    if (auto v = json["pi"]) {
        if (auto d = v->as_double()) {
            std::println("pi      : {}", static_cast<double>(*d));
        }
    }

    if (auto v = json["data"]; v && v->is_null()) {
        std::println("data    : null");
    }

    if (auto pos = json["position"]) {
        if (auto x = (*pos)["x"]) {
            if (auto d = x->as_double()) {
                std::println("pos.x   : {}", static_cast<double>(*d));
            }
        }
        if (auto y = (*pos)["y"]) {
            if (auto d = y->as_double()) {
                std::println("pos.y   : {}", static_cast<double>(*d));
            }
        }
    }

    if (auto colors = json["colors"]) {
        std::println("colors  : {} item(s)", colors->size());
        for (size_t i = 0; i < colors->size(); ++i) {
            if (auto c = (*colors)[i]) {
                if (auto s = c->as_string()) {
                    std::println("  [{}]   : {}", i, *s);
                }
            }
        }
    }
}