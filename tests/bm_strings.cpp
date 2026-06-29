/**
 * @file bm_strings.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Micro-benchmark for string processing (escaping and unescaping)
 * @version 1.2.0
 * @date 2026-06-29
 * * @copyright Copyright (c) 2026
 */

#include <benchmark/benchmark.h>
#include <string>
#include <span>
#include "parser/parser.hpp"
#include "lexer/lexer.hpp"

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

// --- SWAR Tokenizer String Processing ---
// Menguji seberapa efisien Tokenizer::readString menggunakan mask SWAR
// untuk melompati string polos vs mendeteksi backslash (\).

static void BM_SWAR_String_Tokenizer_Plain(benchmark::State& state) {
    std::span<const char> raw(plain_json);
    for (auto _ : state) {
        lexer::SwarPolicy::Engine lexer(raw);
        while (true) {
            auto tok = lexer.next_token();
            if (tok.type_ == models::Token::Type::EndOfFile || lexer.is_error()) break;
            benchmark::DoNotOptimize(tok);
        }
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * plain_json.size());
}
BENCHMARK(BM_SWAR_String_Tokenizer_Plain)->Unit(benchmark::kNanosecond)->MinTime(1.0);

static void BM_SWAR_String_Tokenizer_Escaped(benchmark::State& state) {
    std::span<const char> raw(escaped_json);
    for (auto _ : state) {
        lexer::SwarPolicy::Engine lexer(raw);
        while (true) {
            auto tok = lexer.next_token();
            if (tok.type_ == models::Token::Type::EndOfFile || lexer.is_error()) break;
            benchmark::DoNotOptimize(tok);
        }
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * escaped_json.size());
}
BENCHMARK(BM_SWAR_String_Tokenizer_Escaped)->Unit(benchmark::kNanosecond)->MinTime(1.0);

// --- Parser String Processing ---
// Menguji overhead Parser::unescapeString dan penggunaan alokator Arena.

static void BM_SWAR_String_Parser_Plain(benchmark::State& state) {
    std::span<const char> raw(plain_json);
    lexer::SwarPolicy::Engine lexer(raw);
    auto hint = lexer.pre_scan();

    for (auto _ : state) {
        lexer.reset();
        auto parsed = parser::Parser<parser::DefaultPolicy>::Parse(lexer, hint);
        benchmark::DoNotOptimize(parsed);
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * plain_json.size());
}
BENCHMARK(BM_SWAR_String_Parser_Plain)->Unit(benchmark::kNanosecond)->MinTime(1.0);

static void BM_SWAR_String_Parser_Escaped(benchmark::State& state) {
    std::span<const char> raw(escaped_json);
    lexer::SwarPolicy::Engine lexer(raw);
    auto hint = lexer.pre_scan();

    for (auto _ : state) {
        lexer.reset();
        auto parsed = parser::Parser<parser::DefaultPolicy>::Parse(lexer, hint);
        benchmark::DoNotOptimize(parsed);
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * escaped_json.size());
}
BENCHMARK(BM_SWAR_String_Parser_Escaped)->Unit(benchmark::kNanosecond)->MinTime(1.0);

// --- AVX2 Tokenizer String Processing ---

static void BM_AVX2_String_Tokenizer_Plain(benchmark::State& state) {
    std::span<const char> raw(plain_json);
    for (auto _ : state) {
        lexer::Avx2Policy::Engine lexer(raw);
        while (true) {
            auto tok = lexer.next_token();
            if (tok.type_ == models::Token::Type::EndOfFile || lexer.is_error()) break;
            benchmark::DoNotOptimize(tok);
        }
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * plain_json.size());
}
BENCHMARK(BM_AVX2_String_Tokenizer_Plain)->Unit(benchmark::kNanosecond)->MinTime(1.0);

static void BM_AVX2_String_Tokenizer_Escaped(benchmark::State& state) {
    std::span<const char> raw(escaped_json);
    for (auto _ : state) {
        lexer::Avx2Policy::Engine lexer(raw);
        while (true) {
            auto tok = lexer.next_token();
            if (tok.type_ == models::Token::Type::EndOfFile || lexer.is_error()) break;
            benchmark::DoNotOptimize(tok);
        }
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * escaped_json.size());
}
BENCHMARK(BM_AVX2_String_Tokenizer_Escaped)->Unit(benchmark::kNanosecond)->MinTime(1.0);

// --- Parser String Processing (AVX2) ---

static void BM_AVX2_String_Parser_Plain(benchmark::State& state) {
    std::span<const char> raw(plain_json);
    lexer::Avx2Policy::Engine lexer(raw);
    auto hint = lexer.pre_scan();

    for (auto _ : state) {
        lexer.reset();
        auto parsed = parser::Parser<parser::Avx2Policy>::Parse(lexer, hint);
        benchmark::DoNotOptimize(parsed);
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * plain_json.size());
}
BENCHMARK(BM_AVX2_String_Parser_Plain)->Unit(benchmark::kNanosecond)->MinTime(1.0);

static void BM_AVX2_String_Parser_Escaped(benchmark::State& state) {
    std::span<const char> raw(escaped_json);
    lexer::Avx2Policy::Engine lexer(raw);
    auto hint = lexer.pre_scan();

    for (auto _ : state) {
        lexer.reset();
        auto parsed = parser::Parser<parser::Avx2Policy>::Parse(lexer, hint);
        benchmark::DoNotOptimize(parsed);
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * escaped_json.size());
}
BENCHMARK(BM_AVX2_String_Parser_Escaped)->Unit(benchmark::kNanosecond)->MinTime(1.0);