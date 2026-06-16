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
using namespace zuu::utils;

// ── BENCHMARK 1: Tokenizer murni ──
void Benchmark::SmallTokenizer(benchmark::State& state) {
    Benchmark::Tokenizer(state, std::span<const char>(Benchmark::small));
}
void Benchmark::MediumTokenizer(benchmark::State& state) {
    Benchmark::Tokenizer(state, std::span<const char>(Benchmark::medium));
}
void Benchmark::BigTokenizer(benchmark::State& state) {
    Benchmark::Tokenizer(state, std::span<const char>(Benchmark::big));
}
void Benchmark::HugeTokenizer(benchmark::State& state) {
    Benchmark::Tokenizer(state, std::span<const char>(Benchmark::huge));
}
void Benchmark::NumericTokenizer(benchmark::State& state) {
    Benchmark::Tokenizer(state, std::span<const char>(Benchmark::numeric));
}
void Benchmark::StringTokenizer(benchmark::State& state) {
    Benchmark::Tokenizer(state, std::span<const char>(Benchmark::string));
}
void Benchmark::ArrayTokenizer(benchmark::State& state) {
    Benchmark::Tokenizer(state, std::span<const char>(Benchmark::array));
}
void Benchmark::ObjectTokenizer(benchmark::State& state) {
    Benchmark::Tokenizer(state, std::span<const char>(Benchmark::object));
}
void Benchmark::LiteralTokenizer(benchmark::State& state) {
    Benchmark::Tokenizer(state, std::span<const char>(Benchmark::literal));
}

// ── BENCHMARK 2: Parser murni ──
void Benchmark::SmallParser(benchmark::State& state) {
    Benchmark::Parser(state, std::span<const char>(Benchmark::small));
}
void Benchmark::MediumParser(benchmark::State& state) {
    Benchmark::Parser(state, std::span<const char>(Benchmark::medium));
}
void Benchmark::BigParser(benchmark::State& state) {
    Benchmark::Parser(state, std::span<const char>(Benchmark::big));
}
void Benchmark::HugeParser(benchmark::State& state) {
    Benchmark::Parser(state, std::span<const char>(Benchmark::huge));
}
void Benchmark::NumericParser(benchmark::State& state) {
    Benchmark::Parser(state, std::span<const char>(Benchmark::numeric));
}
void Benchmark::StringParser(benchmark::State& state) {
    Benchmark::Parser(state, std::span<const char>(Benchmark::string));
}
void Benchmark::ArrayParser(benchmark::State& state) {
    Benchmark::Parser(state, std::span<const char>(Benchmark::array));
}
void Benchmark::ObjectParser(benchmark::State& state) {
    Benchmark::Parser(state, std::span<const char>(Benchmark::object));
}
void Benchmark::LiteralParser(benchmark::State& state) {
    Benchmark::Parser(state, std::span<const char>(Benchmark::literal));
}

// ── BENCHMARK 3: Pipeline Penuh (End-to-End) ──
void Benchmark::SmallFullPipeline(benchmark::State& state) {
    Benchmark::FullPipeline(state, std::span<const char>(Benchmark::small));
}
void Benchmark::MediumFullPipeline(benchmark::State& state) {
    Benchmark::FullPipeline(state,
                                        std::span<const char>(Benchmark::medium));
}
void Benchmark::BigFullPipeline(benchmark::State& state) {
    Benchmark::FullPipeline(state, std::span<const char>(Benchmark::big));
}
void Benchmark::HugeFullPipeline(benchmark::State& state) {
    Benchmark::FullPipeline(state, std::span<const char>(Benchmark::huge));
}
void Benchmark::NumericFullPipeline(benchmark::State& state) {
    Benchmark::FullPipeline(state,
                                        std::span<const char>(Benchmark::numeric));
}
void Benchmark::StringFullPipeline(benchmark::State& state) {
    Benchmark::FullPipeline(state,
                                        std::span<const char>(Benchmark::string));
}
void Benchmark::ArrayFullPipeline(benchmark::State& state) {
    Benchmark::FullPipeline(state, std::span<const char>(Benchmark::array));
}
void Benchmark::ObjectFullPipeline(benchmark::State& state) {
    Benchmark::FullPipeline(state,
                                        std::span<const char>(Benchmark::object));
}
void Benchmark::LiteralFullPipeline(benchmark::State& state) {
    Benchmark::FullPipeline(state,
                                        std::span<const char>(Benchmark::literal));
}

// Registrasi Benchmark
BENCHMARK(Benchmark::SmallTokenizer)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::MediumTokenizer)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::BigTokenizer)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::HugeTokenizer)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::NumericTokenizer)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::StringTokenizer)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::ArrayTokenizer)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::ObjectTokenizer)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::LiteralTokenizer)->Unit(benchmark::kNanosecond)->MinTime(2.0);

BENCHMARK(Benchmark::SmallParser)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::MediumParser)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::BigParser)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::HugeParser)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::NumericParser)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::StringParser)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::ArrayParser)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::ObjectParser)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::LiteralParser)->Unit(benchmark::kNanosecond)->MinTime(2.0);

BENCHMARK(Benchmark::SmallFullPipeline)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::MediumFullPipeline)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::BigFullPipeline)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::HugeFullPipeline)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::NumericFullPipeline)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::StringFullPipeline)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::ArrayFullPipeline)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::ObjectFullPipeline)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::LiteralFullPipeline)->Unit(benchmark::kNanosecond)->MinTime(2.0);

BENCHMARK(Benchmark::DomTraversalArray)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::DomTraversalObject)->Unit(benchmark::kNanosecond)->MinTime(2.0);
BENCHMARK(Benchmark::DomTraversalDeep)->Unit(benchmark::kNanosecond)->MinTime(2.0);

// CUSTOM MAIN: Menggantikan BENCHMARK_MAIN() agar file bisa di-load terlebih dahulu
int main(int argc, char** argv) {
    Benchmark::Initialize();
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}