/**
 * @file bm_numbers.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Micro-benchmark for numeric parsing (int and double)
 * @version 1.0.0
 * @date 2026-06-21
 *
 * @copyright Copyright (c) 2026
 */

#include "utils/parse_double.hpp"
#include "utils/parse_int.hpp"
#include "utils/parser.hpp"
#include <benchmark/benchmark.h>
#include <charconv>
#include <string_view>
#include <vector>

using namespace zuu;

// --- Data Uji Numerik ---
// Mewakili berbagai kasus JSON: nol, pecahan standar, eksponensial (besar dan kecil)
const std::vector<std::string_view> double_strings = {
    "0.0", "3.1415926535", "-12345.6789", "6.022e23", "-1.602e-19", "100000000000000.0"};

// Mewakili rentang batas integer: kecil, menengah, hingga 64-bit max/min
const std::vector<std::string_view> int_strings = {
    "0",
    "42",
    "-123456789",
    "9223372036854775807", // INT64_MAX
    "-9223372036854775808" // INT64_MIN
};

// --- Benchmark Double ---

static void
    ParseDouble_Zuu(benchmark::State& state) {
    for (auto _ : state) {
        for (const auto& str : double_strings) {
            auto res = utils::parse<double>(str);
            benchmark::DoNotOptimize(res);
        }
        benchmark::ClobberMemory();
    }
    state.counters["Parsed/s"] =
        benchmark::Counter(static_cast<double>(state.iterations() * double_strings.size()),
                           benchmark::Counter::kIsRate);
}
BENCHMARK(ParseDouble_Zuu)->Unit(benchmark::kNanosecond)->MinTime(1.0);

static void
    Parse_Double_Std(benchmark::State& state) {
    for (auto _ : state) {
        for (const auto& str : double_strings) {
            double value;
            auto res = std::from_chars(str.data(), str.data() + str.size(), value);
            benchmark::DoNotOptimize(value);
            benchmark::DoNotOptimize(res);
        }
        benchmark::ClobberMemory();
    }
    state.counters["Parsed/s"] =
        benchmark::Counter(static_cast<double>(state.iterations() * double_strings.size()),
                           benchmark::Counter::kIsRate);
}
BENCHMARK(Parse_Double_Std)->Unit(benchmark::kNanosecond)->MinTime(1.0);

// --- Benchmark Integer ---

static void
    Parse_Int_Zuu(benchmark::State& state) {
    for (auto _ : state) {
        for (const auto& str : int_strings) {
            auto res = utils::parse<long long>(str);
            benchmark::DoNotOptimize(res);
        }
        benchmark::ClobberMemory();
    }
    state.counters["Parsed/s"] = benchmark::Counter(
        static_cast<double>(state.iterations() * int_strings.size()), benchmark::Counter::kIsRate);
}
BENCHMARK(Parse_Int_Zuu)->Unit(benchmark::kNanosecond)->MinTime(1.0);

static void
    Parse_Int_Std(benchmark::State& state) {
    for (auto _ : state) {
        for (const auto& str : int_strings) {
            long long value;
            auto res = std::from_chars(str.data(), str.data() + str.size(), value);
            benchmark::DoNotOptimize(value);
            benchmark::DoNotOptimize(res);
        }
        benchmark::ClobberMemory();
    }
    state.counters["Parsed/s"] = benchmark::Counter(
        static_cast<double>(state.iterations() * int_strings.size()), benchmark::Counter::kIsRate);
}
BENCHMARK(Parse_Int_Std)->Unit(benchmark::kNanosecond)->MinTime(1.0);