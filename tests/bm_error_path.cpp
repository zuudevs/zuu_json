/**
 * @file bm_error_path.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Micro-benchmark for Error Path (Fast-Fail) and Adversarial Inputs
 * @version 1.0.0
 * @date 2026-06-21
 *
 * @copyright Copyright (c) 2026
 */

#include "zuu_json/json.hpp"
#include <benchmark/benchmark.h>
#include <string>

using namespace zuu;

// --- Data Generator ---

// Membuat array yang bersarang sangat dalam (misal: [[[[[]]]]])
// untuk menguji batas rekursi parser (Stack Overflow susceptibility)
static std::string
    generate_deep_nesting(size_t depth) {
    std::string json;
    json.reserve(depth * 2);
    for (size_t i = 0; i < depth; ++i)
        json += "[";
    for (size_t i = 0; i < depth; ++i)
        json += "]";
    return json;
}

// --- Data Uji Error Path ---
static const std::string valid_json =
    R"({"name": "zuu", "age": 21, "active": true, "tags": ["cpp", "json"]})";

// Kesalahan di akhir file (Trailing Comma) -> Menguji kecepatan bail-out di fase akhir
static const std::string err_trailing_comma =
    R"({"name": "zuu", "age": 21, "active": true, "tags": ["cpp", "json"],})";

// Kesalahan di awal file (Unquoted Key) -> Menguji bail-out secepat mungkin
static const std::string err_unquoted_key = R"({name: "zuu", "age": 21})";

static const std::string deep_json = generate_deep_nesting(1000); // 1000 level kedalaman

// --- Benchmark ---

static void
    ErrorPath_Valid_Baseline(benchmark::State& state) {
    for (auto _ : state) {
        auto doc = Json::parse(valid_json);
        benchmark::DoNotOptimize(doc);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(ErrorPath_Valid_Baseline)->Unit(benchmark::kNanosecond)->MinTime(1.0);

static void
    ErrorPath_Trailing_Comma(benchmark::State& state) {
    for (auto _ : state) {
        auto doc = Json::parse(err_trailing_comma);
        benchmark::DoNotOptimize(doc);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(ErrorPath_Trailing_Comma)->Unit(benchmark::kNanosecond)->MinTime(1.0);

static void
    ErrorPath_Unquoted_Key(benchmark::State& state) {
    for (auto _ : state) {
        auto doc = Json::parse(err_unquoted_key);
        benchmark::DoNotOptimize(doc);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(ErrorPath_Unquoted_Key)->Unit(benchmark::kNanosecond)->MinTime(1.0);

static void
    Adversarial_Deep_Nesting(benchmark::State& state) {
    for (auto _ : state) {
        auto doc = Json::parse(deep_json);
        benchmark::DoNotOptimize(doc);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(Adversarial_Deep_Nesting)->Unit(benchmark::kNanosecond)->MinTime(1.0);