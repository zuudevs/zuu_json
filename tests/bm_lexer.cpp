/**
 * @file bm_tokenizer.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Deep profiling for Lexer (Lexical Analysis)
 * @version 1.1.0
 * @date 2026-06-26
 * * @copyright Copyright (c) 2026
 */

#include <benchmark/benchmark.h>
#include <span>
#include "lexer/lexer.hpp"
#include "utils/fs_util.hpp"

using namespace zuu;

// Macro untuk mengotomatisasi pembuatan benchmark Lexer
#define ZUU_BENCHMARK_TOKENIZER(Name, Filename)                                                    \
    static void BM_SWAR_Tokenizer_##Name(benchmark::State& state) {                                \
        static std::string data = tests::utils::load_sample(Filename).value_or("");                \
        if (data.empty()) {                                                                        \
            state.SkipWithError("Gagal memuat file sampel atau file kosong.");                     \
            return;                                                                                \
        }                                                                                          \
        std::span<const char> raw(data);                                                           \
        for (auto _ : state) {                                                                     \
            auto tokens = lexer::Lexer<lexer::SwarPolicy>::Tokenize(raw);              \
            if (!tokens) {                                                                         \
                state.SkipWithError("Parse failed inside benchmark loop!");                        \
                break;                                                                             \
            }                                                                                      \
            benchmark::DoNotOptimize(tokens);                                                      \
            benchmark::ClobberMemory();												               \
        }                                                                                          \
        state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * data.size());           \
    }                                                                                              \
    BENCHMARK(BM_SWAR_Tokenizer_##Name)->Unit(benchmark::kNanosecond)->MinTime(2.0);               \
                                                                                                   \
    static void BM_AVX2_Tokenizer_##Name(benchmark::State& state) {                                \
        static std::string data = tests::utils::load_sample(Filename).value_or("");                \
        if (data.empty()) {                                                                        \
            state.SkipWithError("Gagal memuat file sampel atau file kosong.");                     \
            return;                                                                                \
        }                                                                                          \
        std::span<const char> raw(data);                                                           \
        for (auto _ : state) {                                                                     \
            auto tokens = lexer::Lexer<lexer::Avx2Policy>::Tokenize(raw);              \
            if (!tokens) {                                                                         \
                state.SkipWithError("Parse failed inside benchmark loop!");                        \
                break;                                                                             \
            }                                                                                      \
            benchmark::DoNotOptimize(tokens);                                                      \
            benchmark::ClobberMemory();												               \
        }                                                                                          \
        state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * data.size());           \
    }                                                                                              \
    BENCHMARK(BM_AVX2_Tokenizer_##Name)->Unit(benchmark::kNanosecond)->MinTime(2.0);

// Registrasi
ZUU_BENCHMARK_TOKENIZER(Small, "github_events.json")
ZUU_BENCHMARK_TOKENIZER(Medium, "twitter.json")
ZUU_BENCHMARK_TOKENIZER(Big, "citm_catalog.json")
ZUU_BENCHMARK_TOKENIZER(Huge, "jeopardy.json")
ZUU_BENCHMARK_TOKENIZER(Numeric, "canada.json")
ZUU_BENCHMARK_TOKENIZER(String, "twitter.json")
ZUU_BENCHMARK_TOKENIZER(Array, "github_events.json")
ZUU_BENCHMARK_TOKENIZER(Object, "citm_catalog.json")
ZUU_BENCHMARK_TOKENIZER(Literal, "literal.json")