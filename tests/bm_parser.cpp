/**
 * @file bm_parser.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Deep profiling for Parser and Arena Allocation
 * @version 1.0.0
 * @date 2026-06-21
 * 
 * @copyright Copyright (c) 2026
 */

#include <benchmark/benchmark.h>
#include <span>
#include "parser/parser.hpp"
#include "parser/policies.hpp"
#include "tokenizer/policies.hpp"
#include "tokenizer/tokenizer.hpp"
#include "utils/fs_util.hpp"

using namespace zuu;

#define ZUU_BENCHMARK_PARSER(Name, Filename)                                                                              \
    static void BM_SWAR_Parser_##Name(benchmark::State& state) {                                                          \
        static std::string data = tests::utils::load_sample(Filename).value_or("");                                       \
        if (data.empty()) {                                                                                               \
            state.SkipWithError("Gagal memuat file sampel.");                                                             \
            return;                                                                                                       \
        }                                                                                                                 \
        auto tokens_opt = tokenizer::Tokenizer<tokenizer::SwarPolicy>::Tokenize(std::span<const char>(data));             \
        if (!tokens_opt) {                                                                                                \
            state.SkipWithError("Setup gagal: Tokenisasi error.");                                                        \
            return;                                                                                                       \
        }                                                                                                                 \
        const auto& tokens = tokens_opt->first;                                                                           \
        const auto& hint = tokens_opt->second;                                                                            \
        const size_t tokens_count = tokens.size();                                                                        \
        for (auto _ : state) {                                                                                            \
            auto parsed = parser::Parser<parser::DefaultPolicy>::Parse(tokens, hint);                                     \
            benchmark::DoNotOptimize(parsed);                                                                             \
            benchmark::ClobberMemory();                                                                                   \
        }                                                                                                                 \
        state.counters["Tokens/s"] = benchmark::Counter(                                                                  \
            static_cast<double>(tokens_count),                                                                            \
			benchmark::Counter::kIsIterationInvariantRate                                                                 \
		);                                                                                                                \
    }                                                                                                                     \
    BENCHMARK(BM_SWAR_Parser_##Name)->Unit(benchmark::kNanosecond)->MinTime(2.0);                                         \
    static void BM_AVX2_Parser_##Name(benchmark::State& state) {                                                          \
        static std::string data = tests::utils::load_sample(Filename).value_or("");                                       \
        if (data.empty()) {                                                                                               \
            state.SkipWithError("Gagal memuat file sampel.");                                                             \
            return;                                                                                                       \
        }                                                                                                                 \
        auto tokens_opt = tokenizer::Tokenizer<tokenizer::Avx2Policy>::Tokenize(std::span<const char>(data));             \
        if (!tokens_opt) {                                                                                                \
            state.SkipWithError("Setup gagal: Tokenisasi error.");                                                        \
            return;                                                                                                       \
        }                                                                                                                 \
        const auto& tokens = tokens_opt->first;                                                                           \
        const auto& hint = tokens_opt->second;                                                                            \
        const size_t tokens_count = tokens.size();                                                                        \
        for (auto _ : state) {                                                                                            \
            auto parsed = parser::Parser<parser::Avx2Policy>::Parse(tokens, hint);                                        \
            benchmark::DoNotOptimize(parsed);                                                                             \
            benchmark::ClobberMemory();                                                                                   \
        }                                                                                                                 \
        state.counters["Tokens/s"] = benchmark::Counter(                                                                  \
            static_cast<double>(tokens_count),                                                                            \
			benchmark::Counter::kIsIterationInvariantRate                                                                 \
		);                                                                                                                \
    }                                                                                                                     \
    BENCHMARK(BM_AVX2_Parser_##Name)->Unit(benchmark::kNanosecond)->MinTime(2.0);

// Registrasi
ZUU_BENCHMARK_PARSER(Small, "github_events.json")
ZUU_BENCHMARK_PARSER(Medium, "twitter.json")
ZUU_BENCHMARK_PARSER(Big, "citm_catalog.json")
ZUU_BENCHMARK_PARSER(Huge, "jeopardy.json")
ZUU_BENCHMARK_PARSER(Numeric, "canada.json")