/**
 * @file bm_main.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Main entry point for Google Benchmark
 * @version 1.0.0
 * @date 2026-06-21
 *
 * @copyright Copyright (c) 2026
 */

#include <benchmark/benchmark.h>

int
    main(int argc, char** argv) {
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}