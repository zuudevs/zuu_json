/**
 * @file bm_storage.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Micro-benchmark for Arena Allocator vs Standard Allocation
 * @version 1.5.0
 * @date 2026-06-29
 * 
 * @copyright Copyright (c) 2026
 */

#include <benchmark/benchmark.h>
#include <cstdint>
#include <string>
#include <vector>
#include "allocators/storage.hpp"
#include "models/json_value.hpp"

using namespace zuu;

// --- Skenario Arena Allocator (Dynamic Chunked) ---
// Mensimulasikan bagaimana Parser yang baru memuat ribuan string ke dalam 
// stack lokal temporer dan menyalinnya sekaligus ke Chunked Arena saat di-seal.

static void BM_Arena_Allocation(benchmark::State& state) {
    const size_t num_elements = state.range(0);
    
    for (auto _ : state) {
        allocators::Storage storage;
        
        // Stack sementara yang mensimulasikan array_stack_ di ParserBase
        std::vector<models::JsonValue> local_stack;
        local_stack.reserve(num_elements);
        
        for (size_t i = 0; i < num_elements; ++i) {
            auto str_idx = storage.commitString("benchmark_string");
            local_stack.push_back(models::JsonValue::String(str_idx));
        }
        
        // Finalisasi array (copy-and-seal ke memori Contiguous di Chunked Arena)
        auto arr_idx = storage.sealArray(local_stack.data(), local_stack.size());
        
        benchmark::DoNotOptimize(arr_idx);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations() * num_elements);
}
BENCHMARK(BM_Arena_Allocation)->RangeMultiplier(4)->Range(1024, 65536)->Unit(benchmark::kNanosecond)->MinTime(1.0);

// --- Skenario Standar Allocation (Baseline) ---

static void BM_Standard_Allocation(benchmark::State& state) {
    const size_t num_elements = state.range(0);
    
    for (auto _ : state) {
        std::vector<std::string> strings;
        std::vector<models::JsonValue> array_elements;
        
        strings.reserve(num_elements);
        array_elements.reserve(num_elements);
        
        for (size_t i = 0; i < num_elements; ++i) {
            strings.emplace_back("benchmark_string");
            array_elements.push_back(models::JsonValue::String(i));
        }
        
        benchmark::DoNotOptimize(strings);
        benchmark::DoNotOptimize(array_elements);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations() * num_elements);
}
BENCHMARK(BM_Standard_Allocation)->RangeMultiplier(4)->Range(1024, 65536)->Unit(benchmark::kNanosecond)->MinTime(1.0);