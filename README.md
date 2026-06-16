<div>
	<h1 align="center">zuu_json</h1>
	<p align="center">A high-performance, header-friendly C++23 JSON library focused on predictable latency and zero-allocation hot paths.</p>
</div>

---

<div align="center">
  <a href="https://en.cppreference.com/w/cpp/23">
    <img src="https://img.shields.io/badge/C%2B%2B-23-blue.svg" alt="C++23">
  </a>

  <a href="https://cmake.org">
    <img src="https://img.shields.io/badge/CMake-3.20%2B-064F8C.svg" alt="CMake">
  </a>

  <a href="LICENSE">
    <img src="https://img.shields.io/badge/license-MIT-green.svg" alt="MIT License">
  </a>

  <a href="#build">
    <img src="https://img.shields.io/badge/platforms-Linux%20%7C%20Windows%20%7C%20macOS-lightgrey.svg" alt="Platforms">
  </a>
</div>

## Overview

`zuu_json` parses JSON into a compact, type-tagged DOM and provides a fluent, exception-free access API. The library is engineered around several performance techniques — a per-byte lookup table for the tokenizer, SWAR (SIMD-within-a-register) scans, a bump-allocated arena, and O(log N) binary-search object lookups — so that the hot path is deterministic and friendly to modern CPU branch predictors and caches.

The public surface is intentionally small: a `Json` document, a `Value` view, an `Error` enum, and a translator.

## Features

- **C++23 standard library only** — `<expected>`, `<span>`, `<print>`, `<bit_cast>`, no third-party dependencies for the core library.
- **Exception-free error model** — every fallible operation returns `std::expected<T, JsonError>`.
- **Type-tagged values** — `JsonValue` packs the type tag and payload into a single 64-bit word (NaN-boxing style).
- **Bump-allocated arena storage** — strings, arrays, and objects live in one contiguous byte buffer.
- **O(log N) object lookups** — object members are sorted by key on seal; lookups use `std::lower_bound`.
- **Performance-engineered tokenizer** — 256-entry lookup table, branch-prediction-friendly, SWAR whitespace scanning.
- **Static analysis ready** — `clang-tidy` integration; memory check via Valgrind on Linux CI.

## Quick Example

```cpp
#include "zuu_json/json.hpp"
#include <print>

int main() {
    constexpr std::string_view json_input = R"({
        "name": "zuu",
        "active": true,
        "age": 21,
        "position": { "x": 20.5, "y": 10.5 },
        "colors": ["white", "blue"]
    })";

    auto result = zuu::Json::parse(json_input);
    if (!result) {
        std::println("Parse error: {}", static_cast<int>(result.error()));
        return 1;
    }

    const auto root = result->root();
    std::println("name  : {}", root["name"].get_string());
    std::println("age   : {}", root["age"].get_integer());
    std::println("pos.x : {}", root["position"]["x"].get_double());

    for (size_t i = 0; i < root["colors"].size(); ++i) {
        std::println("color[{}]: {}", i, root["colors"][i].get_string());
    }
}
```

A larger runnable demo is in [`src/main.cpp`](src/main.cpp).

## Documentation

| Document | Purpose |
| --- | --- |
| [docs/QUICKSTART.md](docs/QUICKSTART.md) | Get up and running in minutes |
| [docs/API.md](docs/API.md) | Full public API reference |
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | System design, data flow, module boundaries |
| [docs/CHANGELOG.md](docs/CHANGELOG.md) | Release history (Keep a Changelog) |
| [docs/ROADMAP.md](docs/ROADMAP.md) | Planned milestones and direction |
| [BUILD.md](BUILD.md) | Environment setup and build instructions |
| [CONTRIBUTING.md](CONTRIBUTING.md) | Contribution workflow and standards |
| [SECURITY.md](SECURITY.md) | Vulnerability disclosure |
| [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) | Community standards |

## Build at a glance

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./bin/zuu_json
```

For full environment setup, see [BUILD.md](BUILD.md).

## Benchmark

Performance measurements from `bin/zuu_json_benchmark` on a 12-core CPU @ 2.1 GHz with `min_time: 2.000` (best of N iterations, values in ns; throughput shown as Mi/s for raw byte rates and M/s / G/s for tokens or lookups).

```bash
2026-06-16T13:41:26+07:00
Running D:\Project\zuu_json\bin\zuu_json_benchmark.exe
Run on (12 X 2096 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 512 KiB (x6)
  L3 Unified 4096 KiB (x2)
--------------------------------------------------------------------------------------------------------
Benchmark                                              Time             CPU   Iterations UserCounters...
--------------------------------------------------------------------------------------------------------
Benchmark::SmallTokenizer/min_time:2.000          416724 ns       412598 ns         6400 bytes_per_second=747.108Mi/s
Benchmark::MediumTokenizer/min_time:2.000        1198951 ns      1195208 ns         2327 bytes_per_second=516.248Mi/s
Benchmark::BigTokenizer/min_time:2.000           5410716 ns      5396586 ns          498 bytes_per_second=314.147Mi/s
Benchmark::HugeTokenizer/min_time:2.000         65968571 ns     65848214 ns           42 bytes_per_second=804.593Mi/s
Benchmark::NumericTokenizer/min_time:2.000       2734880 ns      2713981 ns          996 bytes_per_second=791.007Mi/s
Benchmark::StringTokenizer/min_time:2.000        1197999 ns      1192748 ns         2358 bytes_per_second=517.313Mi/s
Benchmark::ArrayTokenizer/min_time:2.000          424266 ns       426115 ns         6637 bytes_per_second=723.409Mi/s
Benchmark::ObjectTokenizer/min_time:2.000        5235470 ns      5179558 ns          543 bytes_per_second=327.31Mi/s
Benchmark::LiteralTokenizer/min_time:2.000           670 ns          667 ns      4167442 bytes_per_second=587.315Mi/s
Benchmark::SmallParser/min_time:2.000             388788 ns       388009 ns         7168 Tokens/s=159.795M/s
Benchmark::MediumParser/min_time:2.000           3272838 ns      3260551 ns          853 Tokens/s=16.9493M/s
Benchmark::BigParser/min_time:2.000              2988825 ns      2982503 ns          943 Tokens/s=45.5963M/s
Benchmark::HugeParser/min_time:2.000            49184641 ns     48828125 ns           56 Tokens/s=133.282M/s
Benchmark::NumericParser/min_time:2.000         13993068 ns     14054648 ns          199 Tokens/s=23.791M/s
Benchmark::StringParser/min_time:2.000           3252706 ns      3223916 ns          853 Tokens/s=17.1419M/s
Benchmark::ArrayParser/min_time:2.000             390330 ns       390189 ns         7168 Tokens/s=158.902M/s
Benchmark::ObjectParser/min_time:2.000           2982692 ns      2982503 ns          943 Tokens/s=45.5963M/s
Benchmark::LiteralParser/min_time:2.000              249 ns          250 ns     11200000 Tokens/s=296.331M/s
Benchmark::SmallFullPipeline/min_time:2.000       839646 ns       837978 ns         3319 bytes_per_second=367.856Mi/s
Benchmark::MediumFullPipeline/min_time:2.000     4521319 ns      4525688 ns          618 bytes_per_second=136.338Mi/s
Benchmark::BigFullPipeline/min_time:2.000        8342277 ns      8321006 ns          338 bytes_per_second=203.74Mi/s
Benchmark::HugeFullPipeline/min_time:2.000     115480300 ns    115234375 ns           24 bytes_per_second=459.767Mi/s
Benchmark::NumericFullPipeline/min_time:2.000   16831487 ns     16841317 ns          167 bytes_per_second=127.471Mi/s
Benchmark::StringFullPipeline/min_time:2.000     4517022 ns      4500405 ns          618 bytes_per_second=137.104Mi/s
Benchmark::ArrayFullPipeline/min_time:2.000       831214 ns       831854 ns         3381 bytes_per_second=370.564Mi/s
Benchmark::ObjectFullPipeline/min_time:2.000     8407771 ns      8387653 ns          326 bytes_per_second=202.121Mi/s
Benchmark::LiteralFullPipeline/min_time:2.000        961 ns          957 ns      2890323 bytes_per_second=409.633Mi/s
Benchmark::DomTraversalArray/min_time:2.000        0.758 ns        0.758 ns   3733333333 Lookups/s=1.32007G/s
Benchmark::DomTraversalObject/min_time:2.000         156 ns          156 ns     17742574 Lookups/s=12.8308M/s
Benchmark::DomTraversalDeep/min_time:2.000          98.5 ns         98.2 ns     28000000 Lookups/s=40.7273M/s
```

Highlights:

- **Tokenizer throughput** peaks at **~805 Mi/s** on huge inputs and **~747 Mi/s** on small inputs, driven by the per-byte lookup table and SWAR scans.
- **DOM access** is essentially free for arrays (**0.758 ns / 1.32 G lookups/s**) and reaches **~40 M lookups/s** for deep traversal.
- **Full pipeline** (parse + traverse) sustains **~370 Mi/s** on small inputs and **~460 Mi/s** on huge inputs, dominated by the tokenizer on hot paths.

The full benchmark source lives in [`tests/benchmark.cpp`](tests/benchmark.cpp); see [`scripts/benchmark.ps1`](scripts/benchmark.ps1) to reproduce.

## Project Status

The library is in active development. Core parser, tokenizer, arena storage, and DOM access are functional. See [docs/ROADMAP.md](docs/ROADMAP.md) for upcoming work.

## License

[MIT](LICENSE) © 2026 zuu_json contributors.
