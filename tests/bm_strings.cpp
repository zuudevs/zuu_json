/**
 * @file bm_strings.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Micro-benchmark for string processing (escaping and unescaping)
 * @version 1.0.0
 * @date 2026-06-21
 * 
 * @copyright Copyright (c) 2026
 */

#include <benchmark/benchmark.h>
#include <string>
#include <vector>
#include <span>
#include "parser/parser.hpp"
#include "tokenizer/tokenizer.hpp"

using namespace zuu;

// --- Data Generator ---

// Membuat array JSON besar berisi string untuk mengisolasi performa pemrosesan string
static std::string generate_string_array(size_t count, bool escaped) {
    std::string json = "[";
    for (size_t i = 0; i < count; ++i) {
        if (escaped) {
            // Kombinasi escape character standar dan unicode (\u2728 adalah emoji sparkles)
            json += "\"Hello \\n World \\\" test \\\" \\u2728 unicode!\"";
        } else {
            // String polos dengan panjang yang kurang lebih sama agar adil
            json += "\"Hello   World    test      unicode!           \"";
        }
        if (i + 1 < count) json += ",";
    }
    json += "]";
    return json;
}

// Data statis yang di-generate satu kali saat inisialisasi binary
static const std::string plain_json = generate_string_array(20000, false);
static const std::string escaped_json = generate_string_array(20000, true);

// --- Tokenizer String Processing ---
// Menguji seberapa efisien Tokenizer::readString menggunakan mask SWAR
// untuk melompati string polos vs mendeteksi backslash (\).

static void BM_String_Tokenizer_Plain(benchmark::State& state) {
    std::span<const char> raw(plain_json);
    for (auto _ : state) {
        auto tokens = tokenizer::Tokenizer::Tokenize(raw);
        benchmark::DoNotOptimize(tokens);
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * plain_json.size());
}
BENCHMARK(BM_String_Tokenizer_Plain)->Unit(benchmark::kNanosecond)->MinTime(1.0);

static void BM_String_Tokenizer_Escaped(benchmark::State& state) {
    std::span<const char> raw(escaped_json);
    for (auto _ : state) {
        auto tokens = tokenizer::Tokenizer::Tokenize(raw);
        benchmark::DoNotOptimize(tokens);
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * escaped_json.size());
}
BENCHMARK(BM_String_Tokenizer_Escaped)->Unit(benchmark::kNanosecond)->MinTime(1.0);

// --- Parser String Processing ---
// Menguji overhead Parser::unescapeString dan penggunaan alokator Arena.
// Tokenisasi dilakukan di luar loop agar hanya waktu pembentukan DOM/String yang diukur.

static void BM_String_Parser_Plain(benchmark::State& state) {
    auto tokens_opt = tokenizer::Tokenizer::Tokenize(std::span<const char>(plain_json));
    const auto& tokens = tokens_opt->first;
    const auto& hint = tokens_opt->second;

    for (auto _ : state) {
        parser::Parser parser(tokens, hint);
        auto parsed = std::move(parser).result();
        benchmark::DoNotOptimize(parsed);
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * plain_json.size());
}
BENCHMARK(BM_String_Parser_Plain)->Unit(benchmark::kNanosecond)->MinTime(1.0);

static void BM_String_Parser_Escaped(benchmark::State& state) {
    auto tokens_opt = tokenizer::Tokenizer::Tokenize(std::span<const char>(escaped_json));
    const auto& tokens = tokens_opt->first;
    const auto& hint = tokens_opt->second;

    for (auto _ : state) {
        parser::Parser parser(tokens, hint);
        auto parsed = std::move(parser).result();
        benchmark::DoNotOptimize(parsed);
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * escaped_json.size());
}
BENCHMARK(BM_String_Parser_Escaped)->Unit(benchmark::kNanosecond)->MinTime(1.0);