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

#include <benchmark/benchmark.h>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include "parser/parser.hpp"
#include "tokenizer/tokenizer.hpp"
#include <span>
#include <string>
#include <system_error>
#include "zuu_json/json.hpp"
#include "zuu_json/utils/error_translator.hpp"

namespace zuu::utils {

[[nodiscard]] inline std::string 
getSamplePath(const std::string& filename) {
    std::filesystem::path current_file_path = __FILE__;
    std::filesystem::path project_root = current_file_path.parent_path().parent_path();
    return (project_root / "samples" / filename).string();
}

class Benchmark {
  public:
    static void Initialize() {
        if (was_init) {
            return;
        }

        if (auto res = loadFile(getSamplePath("github_events.json")); res) {
            small = *res;
        }
        if (auto res = loadFile(getSamplePath("twitter.json")); res) {
            medium = *res;
        }
        if (auto res = loadFile(getSamplePath("citm_catalog.json")); res) {
            big = *res;
        }
        if (auto res = loadFile(getSamplePath("jeopardy.json")); res) {
            huge = *res;
        }
        if (auto res = loadFile(getSamplePath("canada.json")); res) {
            numeric = *res;
        }
        if (auto res = loadFile(getSamplePath("twitter.json")); res) {
            string = *res;
        }
        if (auto res = loadFile(getSamplePath("github_events.json")); res) {
            array = *res;
        }
        if (auto res = loadFile(getSamplePath("citm_catalog.json")); res) {
            object = *res;
        }
        if (auto res = loadFile(getSamplePath("literal.json")); res) {
            literal = *res;
        }
        was_init = true;
    }

    static void Tokenizer(benchmark::State& state, std::span<const char> raw) {
        const size_t bytes_processed = raw.size();

        if (raw.empty()) {
            state.SkipWithError("File JSON kosong atau path salah! (Tokenizer)");
            return;
        }

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
        if (raw.empty()) {
            state.SkipWithError("File JSON kosong atau path salah! (Parser)");
            return;
        }

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
                state.SkipWithError(std::format("Parser failed!, Error code: {}", utils::TranslateError(parsed.error())));
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
        if (raw.empty()) {
            state.SkipWithError("File JSON kosong atau path salah! (FullPipeline)");
            return;
        }

        const size_t bytes_processed = raw.size();

        for (auto _ : state) {
            auto json = zuu::Json::parse(std::string_view(raw.data(), raw.size()));
            if (!json) {
                state.SkipWithError(std::format("Full Pipeline failed!, Error code: {}", utils::TranslateError(json.error())));
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

    static void DomTraversalArray(benchmark::State& state) {
        if (huge.empty()) {
            state.SkipWithError("File jeopardy.json kosong!");
            return;
        }

        auto json_opt = zuu::Json::parse(huge);
        if (!json_opt) {
            state.SkipWithError("Parsing huge gagal saat setup!");
            return;
        }
        auto& doc = json_opt.value(); 

        for (auto _ : state) {
            auto value = doc.root()[1000]; 
            benchmark::DoNotOptimize(value);
        }
        
        state.counters["Lookups/s"] = benchmark::Counter(
            static_cast<double>(state.iterations()), benchmark::Counter::kIsRate);
    }

    static void DomTraversalObject(benchmark::State& state) {
        if (big.empty()) {
            state.SkipWithError("File citm_catalog.json kosong!");
            return;
        }

        auto json_opt = zuu::Json::parse(big);
        if (!json_opt) {
            state.SkipWithError("Parsing big gagal saat setup!");
            return;
        }
        auto& doc = json_opt.value();

        for (auto _ : state) {
            auto value1 = doc["events"];
            auto value2 = doc["performances"];
            
            benchmark::DoNotOptimize(value1);
            benchmark::DoNotOptimize(value2);
        }

        state.counters["Lookups/s"] = benchmark::Counter(static_cast<double>(state.iterations()) * 2, benchmark::Counter::kIsRate);
    }

    static void DomTraversalDeep(benchmark::State& state) {
        if (medium.empty()) {
            state.SkipWithError("File twitter.json kosong!");
            return;
        }

        auto json_opt = zuu::Json::parse(medium);
        if (!json_opt) {
            state.SkipWithError("Parsing medium gagal saat setup!");
            return;
        }
        auto& doc = json_opt.value();

        for (auto _ : state) {
            auto val = doc["statuses"].value()
                          [0].value()
                          ["user"].value()
                          ["screen_name"].value();
            benchmark::DoNotOptimize(val);
        }
        
        state.counters["Lookups/s"] = benchmark::Counter(
            static_cast<double>(state.iterations()) * 4, benchmark::Counter::kIsRate);
    }

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

    static inline std::string getSamplePath(std::string_view filename) {
        std::filesystem::path cwd = std::filesystem::current_path();
        
        for (int i = 0; i < 4; ++i) {
            std::filesystem::path target = cwd / "samples" / filename;
            if (std::filesystem::exists(target)) {
                return target.string();
            }
            cwd = cwd.parent_path();
        }
        
        return "";
    }

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