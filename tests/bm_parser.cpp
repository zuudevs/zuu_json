/**
 * @file bm_parser.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Deep profiling for Parser and Arena Allocation
 * @version 1.5.0
 * @date 2026-06-29
 * 
 * @copyright Copyright (c) 2026
 */

#include <benchmark/benchmark.h>
#include <span>
#include "parser/parser.hpp"
#include "parser/policies.hpp"
#include "lexer/policies.hpp"
#include "lexer/lexer.hpp"
#include "utils/fs_util.hpp"

using namespace zuu;

#define ZUU_BENCHMARK_PARSER(Name, Filename)                                                                              \
    static void BM_SWAR_Parser_##Name(benchmark::State& state) {                                                          \
        static std::string data = tests::utils::load_sample(Filename).value_or("");                                       \
        if (data.empty()) {                                                                                               \
            state.SkipWithError("Gagal memuat file sampel.");                                                             \
            return;                                                                                                       \
        }                                                                                                                 \
        auto raw = std::span<const char>(data);																			  \
        lexer::SwarPolicy::Engine tokenizer(raw);                                                                     \
        for (auto _ : state) {                                                                                            \
            tokenizer.reset();                                                                                            \
            auto parsed = parser::Parser<parser::DefaultPolicy>::Parse(tokenizer);                                  \
            benchmark::DoNotOptimize(parsed);                                                                             \
            benchmark::ClobberMemory();                                                                                   \
        }                                                                                                                 \
        state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * data.size());                                  \
    }                                                                                                                     \
    BENCHMARK(BM_SWAR_Parser_##Name)->Unit(benchmark::kNanosecond)->MinTime(2.0);                                         \
    static void BM_AVX2_Parser_##Name(benchmark::State& state) {                                                          \
        static std::string data = tests::utils::load_sample(Filename).value_or("");                                       \
        if (data.empty()) {                                                                                               \
            state.SkipWithError("Gagal memuat file sampel.");                                                             \
            return;                                                                                                       \
        }                                                                                                                 \
        auto raw = std::span<const char>(data);																			  \
        lexer::Avx2Policy::Engine tokenizer(raw);                                                                     \
        for (auto _ : state) {                                                                                            \
            tokenizer.reset();                                                                                            \
            auto parsed = parser::Parser<parser::Avx2Policy>::Parse(tokenizer);                                     \
            benchmark::DoNotOptimize(parsed);                                                                             \
            benchmark::ClobberMemory();                                                                                   \
        }                                                                                                                 \
        state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * data.size());                                  \
    }                                                                                                                     \
    BENCHMARK(BM_AVX2_Parser_##Name)->Unit(benchmark::kNanosecond)->MinTime(2.0);

// Registrasi
ZUU_BENCHMARK_PARSER(Small, "github_events.json")
ZUU_BENCHMARK_PARSER(Medium, "twitter.json")
ZUU_BENCHMARK_PARSER(Big, "citm_catalog.json")
ZUU_BENCHMARK_PARSER(Huge, "jeopardy.json")
ZUU_BENCHMARK_PARSER(Numeric, "canada.json")