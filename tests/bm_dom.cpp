/**
 * @file bm_dom.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Deep profiling for DOM Traversal and Data Retrieval
 * @version 1.0.0
 * @date 2026-06-21
 * * @copyright Copyright (c) 2026
 */

#include "utils/fs_util.hpp"
#include "zuu_json/json.hpp"
#include <benchmark/benchmark.h>

using namespace zuu;

static void
    BM_DOM_ArrayAccess(benchmark::State& state) {
    static std::string data = tests::utils::load_sample("jeopardy.json").value_or("");
    static auto doc_opt = Json::parse(data);
    if (!doc_opt) {
        state.SkipWithError("Parse gagal di fase setup.");
        return;
    }

    auto root = doc_opt->root();
    for (auto _ : state) {
        auto value = root[1000]; // Akses elemen ke-1000 (O(1))
        benchmark::DoNotOptimize(value);
    }
    state.counters["Lookups/s"] =
        benchmark::Counter(static_cast<double>(state.iterations()), benchmark::Counter::kIsRate);
}
BENCHMARK(BM_DOM_ArrayAccess)->Unit(benchmark::kNanosecond)->MinTime(2.0);

static void
    BM_DOM_ObjectLookup(benchmark::State& state) {
    static std::string data = tests::utils::load_sample("citm_catalog.json").value_or("");
    static auto doc_opt = Json::parse(data);
    if (!doc_opt) {
        state.SkipWithError("Parse gagal di fase setup.");
        return;
    }

    doc_opt->sort();
    auto root = doc_opt->root();
    for (auto _ : state) {
        auto value1 = root["events"];
        auto value2 = root["performances"];
        benchmark::DoNotOptimize(value1);
        benchmark::DoNotOptimize(value2);
    }
    state.counters["Lookups/s"] = benchmark::Counter(static_cast<double>(state.iterations()) * 2,
                                                     benchmark::Counter::kIsRate);
}
BENCHMARK(BM_DOM_ObjectLookup)->Unit(benchmark::kNanosecond)->MinTime(2.0);

static void
    BM_DOM_DeepTraversal(benchmark::State& state) {
    static std::string data = tests::utils::load_sample("twitter.json").value_or("");
    static auto doc_opt = Json::parse(data);
    if (!doc_opt) {
        state.SkipWithError("Parse gagal di fase setup.");
        return;
    }

    doc_opt->sort();
    auto root = doc_opt->root();
    for (auto _ : state) {
        // Simulasi akses bertingkat tinggi
        auto val = root["statuses"][0]["user"]["screen_name"].get_string();
        benchmark::DoNotOptimize(val);
    }
    state.counters["Lookups/s"] = benchmark::Counter(static_cast<double>(state.iterations()) * 4,
                                                     benchmark::Counter::kIsRate);
}
BENCHMARK(BM_DOM_DeepTraversal)->Unit(benchmark::kNanosecond)->MinTime(2.0);