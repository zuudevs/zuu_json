/**
 * @file benchmark.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-11
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "parser/parser.hpp"
#include "tokenizer/tokenizer.hpp"
#include "zuu_json/json.hpp"
#include <benchmark/benchmark.h>
#include <expected>
#include <fstream>
#include <print>
#include <span>
#include <string>
#include <system_error>

namespace zuu::utils {

class Benchmark {
  public:
    static void Initialize() {
        if (was_init) {
            return;
        }

        if (auto res = loadFile("samples/github_events.json"); res) {
            small = *res;
        }
        if (auto res = loadFile("samples/twitter.json"); res) {
            medium = *res;
        }
        if (auto res = loadFile("samples/citm_catalog.json"); res) {
            big = *res;
        }
        if (auto res = loadFile("samples/jeopardy.json"); res) {
            huge = *res;
        }
        if (auto res = loadFile("samples/canada.json"); res) {
            numeric = *res;
        }
        if (auto res = loadFile("samples/twitter.json"); res) {
            string = *res;
        }
        if (auto res = loadFile("samples/github_events.json"); res) {
            array = *res;
        }
        if (auto res = loadFile("samples/citm_catalog.json"); res) {
            object = *res;
        }
        if (auto res = loadFile("samples/literal.json"); res) {
            literal = *res;
        }
    }

    static void Tokenizer(benchmark::State& state, std::span<const char> raw) {
        const size_t bytes_processed = raw.size();

        for (auto _ : state) {
            auto tokens = zuu::tokenizer::Tokenizer::Tokenize(raw);
            if (!tokens) {
                state.SkipWithError("Tokenizer failed!");
                break;
            }
            benchmark::DoNotOptimize(tokens);
        }
        state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * bytes_processed);
    }

    static void SmallTokenizer(benchmark::State& state);
    static void MediumTokenizer(benchmark::State& state);
    static void BigTokenizer(benchmark::State& state);
    static void HugeTokenizer(benchmark::State& state);
    static void NumericTokenizer(benchmark::State& state);
    static void StringTokenizer(benchmark::State& state);
    static void ArrayTokenizer(benchmark::State& state);
    static void ObjectTokenizer(benchmark::State& state);
    static void LiteralTokenizer(benchmark::State& state);

    static void Parser(benchmark::State& state, std::span<const char> raw) {
        auto tokens_result = zuu::tokenizer::Tokenizer::Tokenize(raw);

        if (!tokens_result) {
            state.SkipWithError("Tokenizer failed during setup!");
            return;
        }
        const size_t nodes_count = tokens_result->size();

        for (auto _ : state) {
            zuu::parser::Parser parser(tokens_result.value());
            auto parsed = std::move(parser).result();

            if (!parsed) {
                state.SkipWithError("Parser failed!");
                break;
            }
            benchmark::DoNotOptimize(parsed);
        }
        state.counters["Tokens/s"] = benchmark::Counter(
            static_cast<double>(nodes_count), benchmark::Counter::kIsIterationInvariantRate);
    }

    static void SmallParser(benchmark::State& state);
    static void MediumParser(benchmark::State& state);
    static void BigParser(benchmark::State& state);
    static void HugeParser(benchmark::State& state);
    static void NumericParser(benchmark::State& state);
    static void StringParser(benchmark::State& state);
    static void ArrayParser(benchmark::State& state);
    static void ObjectParser(benchmark::State& state);
    static void LiteralParser(benchmark::State& state);

    static void FullPipeline(benchmark::State& state, std::span<const char> raw) {
        const size_t bytes_processed = raw.size();

        for (auto _ : state) {
            auto json = zuu::Json::parse(std::string_view(raw.data(), raw.size()));
            if (!json) {
                state.
				SkipWithError("Full Pipeline failed!");
                break;
            }
            benchmark::DoNotOptimize(json);
        }
        state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * bytes_processed);
    }

    static void SmallFullPipeline(benchmark::State& state);
    static void MediumFullPipeline(benchmark::State& state);
    static void BigFullPipeline(benchmark::State& state);
    static void HugeFullPipeline(benchmark::State& state);
    static void NumericFullPipeline(benchmark::State& state);
    static void StringFullPipeline(benchmark::State& state);
    static void ArrayFullPipeline(benchmark::State& state);
    static void ObjectFullPipeline(benchmark::State& state);
    static void LiteralFullPipeline(benchmark::State& state);

  private:
    static inline std::string small;
    static inline std::string medium;
    static inline std::string big;
    static inline std::string huge;
    static inline std::string numeric;
    static inline std::string string;
    static inline std::string object;
    static inline std::string array;
    static inline std::string literal;
    static inline bool was_init{false};

    [[nodiscard]] static std::expected<std::string, std::errc>
    loadFile(std::string_view filepath) noexcept {
        std::ifstream file(std::string(filepath), std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            return std::unexpected{std::errc::no_such_file_or_directory};
        }

        file.seekg(0, std::ios::end);
        const auto size = file.tellg();
        if (size < 0) {
            return std::unexpected{std::errc::io_error};
        }

        std::string result(static_cast<std::size_t>(size), '\0');
        file.seekg(0, std::ios::beg);

        file.read(result.data(), static_cast<std::streamsize>(result.size()));
        if (!file && !file.eof()) {
            return std::unexpected{std::errc::io_error};
        }

        result.resize(static_cast<std::size_t>(file.gcount()));
        return result;
    }
};

} // namespace zuu::utils