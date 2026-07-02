/**
 * @file bm_pipeline.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief End-to-end parsing pipeline profiling
 * @version 1.1.0
 * @date 2026-06-26
 * * @copyright Copyright (c) 2026
 */

#include "utils/fs_util.hpp"
#include "zuu_json/core/engine.hpp"
#include "zuu_json/json.hpp"
#include "zuu_json/models/policies.hpp"
#include <benchmark/benchmark.h>

using namespace zuu;

#define ZUU_BENCHMARK_PIPELINE(Name, Filename)                                                     \
    static void SWAR_Pipeline_##Name(benchmark::State& state) {                                    \
        static std::string data = tests::utils::load_sample(Filename).value_or("");                \
        if (data.empty()) {                                                                        \
            state.SkipWithError("Gagal memuat file sampel.");                                      \
            return;                                                                                \
        }                                                                                          \
        for (auto _ : state) {                                                                     \
            auto doc = Json::parse(data);                                                          \
            if (!doc) {                                                                            \
                state.SkipWithError("Parse failed inside benchmark loop!");                        \
                break;                                                                             \
            }                                                                                      \
            benchmark::DoNotOptimize(doc);                                                         \
            benchmark::ClobberMemory();                                                            \
        }                                                                                          \
        state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * data.size());           \
    }                                                                                              \
    BENCHMARK(SWAR_Pipeline_##Name)->Unit(benchmark::kNanosecond)->MinTime(2.0);                   \
    static void AVX2_Pipeline_##Name(benchmark::State& state) {                                    \
        static std::string data = tests::utils::load_sample(Filename).value_or("");                \
        auto avx2_policy = models::Policy{core::TokenizerEngine::Avx2, core::ParserEngine::Avx2};  \
        if (data.empty()) {                                                                        \
            state.SkipWithError("Gagal memuat file sampel.");                                      \
            return;                                                                                \
        }                                                                                          \
        for (auto _ : state) {                                                                     \
            auto doc = Json::parse(data, avx2_policy);                                             \
            if (!doc) {                                                                            \
                state.SkipWithError("Parse failed inside benchmark loop!");                        \
                break;                                                                             \
            }                                                                                      \
            benchmark::DoNotOptimize(doc);                                                         \
            benchmark::ClobberMemory();                                                            \
        }                                                                                          \
        state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * data.size());           \
    }                                                                                              \
    BENCHMARK(AVX2_Pipeline_##Name)->Unit(benchmark::kNanosecond)->MinTime(2.0);

// Registrasi
ZUU_BENCHMARK_PIPELINE(Small, "github_events.json")
ZUU_BENCHMARK_PIPELINE(Medium, "twitter.json")
ZUU_BENCHMARK_PIPELINE(Big, "citm_catalog.json")
ZUU_BENCHMARK_PIPELINE(Huge, "jeopardy.json")