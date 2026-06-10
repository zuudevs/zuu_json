/**
 * @file benchmark.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief High-Precision Benchmarks JSON Parser
 * @version 0.1.0
 * @date 2026-06-05
 *
 * @copyright Copyright (c) 2026
 */

#define CPP_JSON_DEBUG

#include "parser/parser.hpp"
#include "tokenizer/tokenizer.hpp"
#include "zuu_json/json.hpp"
#include <benchmark/benchmark.h>
#include <string>

static constexpr std::string_view kJsonData =
    R"({"name": "benchmark", "values": [1, 2, 3, 4, 5], "nested": {"x": 10.5, "y": -20.5}, "active": true, "null_val": null})";

// ── BENCHMARK 1: Tokenizer murni ──
static void BM_Tokenizer(benchmark::State& state) {
    const auto raw = std::span<const char>(kJsonData.data(), kJsonData.size());
    const size_t bytes_processed = raw.size();

    while (state.KeepRunning()) {
        auto tokens = zuu::tokenizer::Tokenizer::Tokenize(raw);
        if (!tokens) {
            std::string err_msg = "Tokenizer failed during execution! Error Code: " +
                                  std::to_string(static_cast<int>(tokens.error()));
            state.SkipWithError(err_msg.c_str());
            break;
        }
        benchmark::DoNotOptimize(tokens);
    }

    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * bytes_processed);
}

// ── BENCHMARK 2: Parser murni ──
static void BM_ParserOnly(benchmark::State& state) {
    const auto raw = std::span<const char>(kJsonData.data(), kJsonData.size());

    auto tokens_result = zuu::tokenizer::Tokenizer::Tokenize(raw);
    if (!tokens_result) {
        std::string err_msg = "Tokenizer failed during setup! Error Code: " +
                              std::to_string(static_cast<int>(tokens_result.error()));
        state.SkipWithError(err_msg.c_str());
        return;
    }

    const size_t nodes_count = tokens_result->size();

    for (auto _ : state) {
        zuu::parser::Parser parser(tokens_result.value());
        auto parsed = std::move(parser).result();

        if (!parsed) {
            std::string err_msg = "Parser failed during DOM Construction! Error Code: " +
                                  std::to_string(static_cast<int>(parsed.error()));
            state.SkipWithError(err_msg.c_str());
            break;
        }
        benchmark::DoNotOptimize(parsed);
    }

    state.counters["Tokens/s"] = benchmark::Counter(static_cast<double>(nodes_count),
                                                    benchmark::Counter::kIsIterationInvariantRate);
}

// ── BENCHMARK 3: Pipeline Penuh (End-to-End) ──
static void BM_FullPipeline(benchmark::State& state) {
    const size_t bytes_processed = kJsonData.size();

    for (auto _ : state) {
        auto json = zuu::Json::parse(kJsonData);
        if (!json) {
            std::string err_msg = "Full Pipeline failed! Error Code: " +
                                  std::to_string(static_cast<int>(json.error()));
            state.SkipWithError(err_msg.c_str());
            break;
        }
        benchmark::DoNotOptimize(json);
    }

    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * bytes_processed);
}

// Registrasi Benchmark: Minimal 2 detik dan format ke Mikrodetik
BENCHMARK(BM_Tokenizer)->Unit(benchmark::kMicrosecond)->MinTime(2.0);
BENCHMARK(BM_ParserOnly)->Unit(benchmark::kMicrosecond)->MinTime(2.0);
BENCHMARK(BM_FullPipeline)->Unit(benchmark::kMicrosecond)->MinTime(2.0);

BENCHMARK_MAIN();