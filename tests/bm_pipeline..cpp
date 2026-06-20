/**
 * @file bm_pipeline.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief End-to-end parsing pipeline profiling
 * @version 1.0.0
 * @date 2026-06-21
 * 
 * @copyright Copyright (c) 2026
 */

#include <benchmark/benchmark.h>
#include "zuu_json/json.hpp"
#include "utils/fs_util.hpp"

using namespace zuu;

#define ZUU_BENCHMARK_PIPELINE(Name, Filename)                                                     \
    static void BM_Pipeline_##Name(benchmark::State& state) {                                      \
        static std::string data = tests::utils::load_sample(Filename).value_or("");                \
        if (data.empty()) {                                                                        \
            state.SkipWithError("Gagal memuat file sampel.");                                      \
            return;                                                                                \
        }                                                                                          \
        for (auto _ : state) {                                                                     \
            auto doc = Json::parse(data);                                                          \
            benchmark::DoNotOptimize(doc);                                                         \
            benchmark::ClobberMemory();                                                            \
        }                                                                                          \
        state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * data.size());           \
    }                                                                                              \
    BENCHMARK(BM_Pipeline_##Name)->Unit(benchmark::kNanosecond)->MinTime(2.0);

// Registrasi
ZUU_BENCHMARK_PIPELINE(Small, "github_events.json")
ZUU_BENCHMARK_PIPELINE(Medium, "twitter.json")
ZUU_BENCHMARK_PIPELINE(Big, "citm_catalog.json")
ZUU_BENCHMARK_PIPELINE(Huge, "jeopardy.json")