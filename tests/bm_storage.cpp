/**
 * @file bm_storage.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Micro-benchmark for Arena Allocator vs Standard Allocation
 * @version 1.0.0
 * @date 2026-06-21
 * 
 * @copyright Copyright (c) 2026
 */

#include <benchmark/benchmark.h>
#include <cstdint>
#include <string>
#include <vector>
#include "models/storage.hpp"
#include "models/json_value.hpp"

using namespace zuu;

// --- Skenario Arena Allocator (zuu_json) ---
// Mensimulasikan bagaimana Parser memuat ribuan string ke dalam Storage
// menggunakan single-allocation bump pointer.

static void BM_Arena_Allocation(benchmark::State& state) {
    const size_t num_elements = state.range(0);
    
    // Menyimulasikan Hint yang dihasilkan oleh Tokenizer
    traits::HintTrait<models::Token> hint{};
    hint.array_count_ = 1;
    hint.string_count_ = num_elements;
    hint.comma_count_ = num_elements;
    hint.string_escape_bytes_ = num_elements * sizeof(uint16_t);
    
    for (auto _ : state) {
        models::Storage storage(hint);
        
        for (size_t i = 0; i < num_elements; ++i) {
            // Interning string (tanpa alokasi heap tambahan)
            auto str_idx = storage.commitString("benchmark_string");
            
            // Push elemen ke array (bump pointer)
            storage.pushArrayElement(models::JsonValue::String(str_idx));
        }
        
        // Finalisasi array
        auto arr_idx = storage.sealArray(0);
        
        benchmark::DoNotOptimize(arr_idx);
        benchmark::ClobberMemory();
    }
    // Metrik: Berapa juta node/alokasi yang bisa diproses per detik
    state.SetItemsProcessed(state.iterations() * num_elements);
}
BENCHMARK(BM_Arena_Allocation)->RangeMultiplier(4)->Range(1024, 65536)->Unit(benchmark::kNanosecond)->MinTime(1.0);

// --- Skenario Standar Allocation (Baseline) ---
// Mensimulasikan pendekatan parser konvensional yang menggunakan std::vector
// dan alokasi dinamis untuk menyimpan string dan node.

static void BM_Standard_Allocation(benchmark::State& state) {
    const size_t num_elements = state.range(0);
    
    for (auto _ : state) {
        std::vector<std::string> strings;
        std::vector<models::JsonValue> array_elements;
        
        // Fair play: Kita asumsikan parser konvensional juga tahu kapasitasnya (reserve)
        strings.reserve(num_elements);
        array_elements.reserve(num_elements);
        
        for (size_t i = 0; i < num_elements; ++i) {
            // Alokasi string konvensional (berpotensi memicu heap allocation jika > SSO limit)
            // String "benchmark_string" adalah 16 bytes, sering kali berada di batas SSO (Small String Optimization)
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