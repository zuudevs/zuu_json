/**
 * @file bm_serializer.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Micro-benchmark for JSON Serialization Engine
 * @version 1.0.0
 * @date 2026-06-21
 * 
 * @copyright Copyright (c) 2026
 */

#include <benchmark/benchmark.h>
#include "zuu_json/json.hpp"
#include "utils/fs_util.hpp"

using namespace zuu;

#define ZUU_BENCHMARK_SERIALIZER(Name, Filename)                                                   \
    static void BM_Serializer_##Name(benchmark::State& state) {                                    \
        static std::string data = tests::utils::load_sample(Filename).value_or("");                \
        static auto doc_opt = Json::parse(data);                                                   \
        if (!doc_opt) {                                                                            \
            state.SkipWithError("Parse gagal di fase setup.");                                     \
            return;                                                                                \
        }                                                                                          \
                                                                                                   \
        for (auto _ : state) {                                                                     \
            /* Indent -1: Minified output (pure write speed) */                                    \
            std::string output = doc_opt->dump(-1);                                                \
            benchmark::DoNotOptimize(output);                                                      \
            benchmark::ClobberMemory();                                                            \
        }                                                                                          \
        state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * data.size());           \
    }                                                                                              \
    BENCHMARK(BM_Serializer_##Name)->Unit(benchmark::kNanosecond)->MinTime(2.0);

// Registrasi
ZUU_BENCHMARK_SERIALIZER(Small, "github_events.json")
ZUU_BENCHMARK_SERIALIZER(Medium, "twitter.json")
ZUU_BENCHMARK_SERIALIZER(Big, "citm_catalog.json")