/**
 * @file benchmark.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief High-Precision Benchmarks JSON Parser
 * @version 0.1.0
 * @date 2026-06-11
 *
 * @copyright Copyright (c) 2026
 */

#include "utils/benchmark.hpp"

// ── BENCHMARK 1: Tokenizer murni ──
void zuu::utils::Benchmark::SmallTokenizer(benchmark::State& state) {
    zuu::utils::Benchmark::Tokenizer(state, std::span<const char>(zuu::utils::Benchmark::small));
}
void zuu::utils::Benchmark::MediumTokenizer(benchmark::State& state) {
    zuu::utils::Benchmark::Tokenizer(state, std::span<const char>(zuu::utils::Benchmark::medium));
}
void zuu::utils::Benchmark::BigTokenizer(benchmark::State& state) {
    zuu::utils::Benchmark::Tokenizer(state, std::span<const char>(zuu::utils::Benchmark::big));
}
void zuu::utils::Benchmark::HugeTokenizer(benchmark::State& state) {
    zuu::utils::Benchmark::Tokenizer(state, std::span<const char>(zuu::utils::Benchmark::huge));
}
void zuu::utils::Benchmark::NumericTokenizer(benchmark::State& state) {
    zuu::utils::Benchmark::Tokenizer(state, std::span<const char>(zuu::utils::Benchmark::numeric));
}
void zuu::utils::Benchmark::StringTokenizer(benchmark::State& state) {
    zuu::utils::Benchmark::Tokenizer(state, std::span<const char>(zuu::utils::Benchmark::string));
}
void zuu::utils::Benchmark::ArrayTokenizer(benchmark::State& state) {
    zuu::utils::Benchmark::Tokenizer(state, std::span<const char>(zuu::utils::Benchmark::array));
}
void zuu::utils::Benchmark::ObjectTokenizer(benchmark::State& state) {
    zuu::utils::Benchmark::Tokenizer(state, std::span<const char>(zuu::utils::Benchmark::object));
}
void zuu::utils::Benchmark::LiteralTokenizer(benchmark::State& state) {
    zuu::utils::Benchmark::Tokenizer(state, std::span<const char>(zuu::utils::Benchmark::literal));
}

// ── BENCHMARK 2: Parser murni ──
void zuu::utils::Benchmark::SmallParser(benchmark::State& state) {
    Benchmark::Initialize();
    zuu::utils::Benchmark::Parser(state, std::span<const char>(zuu::utils::Benchmark::small));
}
void zuu::utils::Benchmark::MediumParser(benchmark::State& state) {
    zuu::utils::Benchmark::Parser(state, std::span<const char>(zuu::utils::Benchmark::medium));
}
void zuu::utils::Benchmark::BigParser(benchmark::State& state) {
    zuu::utils::Benchmark::Parser(state, std::span<const char>(zuu::utils::Benchmark::big));
}
void zuu::utils::Benchmark::HugeParser(benchmark::State& state) {
    zuu::utils::Benchmark::Parser(state, std::span<const char>(zuu::utils::Benchmark::huge));
}
void zuu::utils::Benchmark::NumericParser(benchmark::State& state) {
    zuu::utils::Benchmark::Parser(state, std::span<const char>(zuu::utils::Benchmark::numeric));
}
void zuu::utils::Benchmark::StringParser(benchmark::State& state) {
    zuu::utils::Benchmark::Parser(state, std::span<const char>(zuu::utils::Benchmark::string));
}
void zuu::utils::Benchmark::ArrayParser(benchmark::State& state) {
    zuu::utils::Benchmark::Parser(state, std::span<const char>(zuu::utils::Benchmark::array));
}
void zuu::utils::Benchmark::ObjectParser(benchmark::State& state) {
    zuu::utils::Benchmark::Parser(state, std::span<const char>(zuu::utils::Benchmark::object));
}
void zuu::utils::Benchmark::LiteralParser(benchmark::State& state) {
    zuu::utils::Benchmark::Parser(state, std::span<const char>(zuu::utils::Benchmark::literal));
}

// ── BENCHMARK 3: Pipeline Penuh (End-to-End) ──
void zuu::utils::Benchmark::SmallFullPipeline(benchmark::State& state) {
    zuu::utils::Benchmark::FullPipeline(state, std::span<const char>(zuu::utils::Benchmark::small));
}
void zuu::utils::Benchmark::MediumFullPipeline(benchmark::State& state) {
    zuu::utils::Benchmark::FullPipeline(state,
                                        std::span<const char>(zuu::utils::Benchmark::medium));
}
void zuu::utils::Benchmark::BigFullPipeline(benchmark::State& state) {
    zuu::utils::Benchmark::FullPipeline(state, std::span<const char>(zuu::utils::Benchmark::big));
}
void zuu::utils::Benchmark::HugeFullPipeline(benchmark::State& state) {
    zuu::utils::Benchmark::FullPipeline(state, std::span<const char>(zuu::utils::Benchmark::huge));
}
void zuu::utils::Benchmark::NumericFullPipeline(benchmark::State& state) {
    zuu::utils::Benchmark::FullPipeline(state,
                                        std::span<const char>(zuu::utils::Benchmark::numeric));
}
void zuu::utils::Benchmark::StringFullPipeline(benchmark::State& state) {
    zuu::utils::Benchmark::FullPipeline(state,
                                        std::span<const char>(zuu::utils::Benchmark::string));
}
void zuu::utils::Benchmark::ArrayFullPipeline(benchmark::State& state) {
    zuu::utils::Benchmark::FullPipeline(state, std::span<const char>(zuu::utils::Benchmark::array));
}
void zuu::utils::Benchmark::ObjectFullPipeline(benchmark::State& state) {
    zuu::utils::Benchmark::FullPipeline(state,
                                        std::span<const char>(zuu::utils::Benchmark::object));
}
void zuu::utils::Benchmark::LiteralFullPipeline(benchmark::State& state) {
    zuu::utils::Benchmark::FullPipeline(state,
                                        std::span<const char>(zuu::utils::Benchmark::literal));
}

// // ── BENCHMARK 4: Spesifik Pause/Resume ──
// static void BM_SpecificBlock_PauseResume(benchmark::State& state) {
//     for (auto _ : state) {
//         state.PauseTiming();
//         std::string json_copy = Sample::sample;
//         state.ResumeTiming();

//         auto json = zuu::Json::parse(json_copy);

//         state.PauseTiming();
//         benchmark::DoNotOptimize(json);
//         state.ResumeTiming();
//     }
// }

// Registrasi Benchmark
BENCHMARK(zuu::utils::Benchmark::SmallTokenizer)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::MediumTokenizer)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::BigTokenizer)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::HugeTokenizer)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::NumericTokenizer)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::StringTokenizer)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::ArrayTokenizer)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::ObjectTokenizer)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::LiteralTokenizer)->Unit(benchmark::kNanosecond)->MinTime(2.0);

BENCHMARK(zuu::utils::Benchmark::SmallParser)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::MediumParser)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::BigParser)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::HugeParser)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::NumericParser)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::StringParser)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::ArrayParser)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::ObjectParser)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::LiteralParser)->Unit(benchmark::kNanosecond)->MinTime(2.0);

BENCHMARK(zuu::utils::Benchmark::SmallFullPipeline)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::MediumFullPipeline)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::BigFullPipeline)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::HugeFullPipeline)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::NumericFullPipeline)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::StringFullPipeline)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::ArrayFullPipeline)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::ObjectFullPipeline)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(zuu::utils::Benchmark::LiteralFullPipeline)->Unit(benchmark::kNanosecond)->MinTime(2.0);

// BENCHMARK(BM_SpecificBlock_PauseResume)->Unit(benchmark::kMicrosecond);

// CUSTOM MAIN: Menggantikan BENCHMARK_MAIN() agar file bisa di-load terlebih dahulu
int main(int argc, char** argv) {
    zuu::utils::Benchmark::Initialize();
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();

    return 0;
}