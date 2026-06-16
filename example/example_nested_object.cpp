/**
 * @file example_nested_object.cpp
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
    std::string_view json_input = R"({
        "komputer": {
            "cpu": "Ryzen 5",
            "ram": 16,
            "gpu": {
                "merk": "NVIDIA",
                "vram": 8
            }
        }
    })";

    auto doc = zuu::Json::parse(json_input).value();
    auto root = doc.root();

    // Optional Chaining: Akses langsung ke level dalam!
    // Tidak akan crash meski "komputer" atau "gpu" tidak ada.
    std::string_view merk_gpu = root["komputer"]["gpu"]["merk"].get_string("Unknown GPU");
    long long vram = root["komputer"]["gpu"]["vram"].get_integer(0);

    std::println("GPU: {} ({} GB VRAM)", merk_gpu, vram);

    // Cek apakah key tertentu ada
    if (root["komputer"].contains("ram")) {
        std::println("Info RAM tersedia: {} GB", root["komputer"]["ram"].get_integer());
    }
}
