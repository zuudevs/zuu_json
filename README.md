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

    for (unsigned long long i = 0; i < root["colors"].size(); ++i) {
        std::println("color[{}]: {}", i, root["colors"][i].get_string());
    }
}
```

A larger runnable demo is in [`src/main.cpp`](src/main.cpp).

## Documentation

| Document                                     | Purpose                                     |
| -------------------------------------------- | ------------------------------------------- |
| [docs/QUICKSTART.md](docs/QUICKSTART.md)     | Get up and running in minutes               |
| [docs/API.md](docs/API.md)                   | Full public API reference                   |
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | System design, data flow, module boundaries |
| [docs/CHANGELOG.md](docs/CHANGELOG.md)       | Release history (Keep a Changelog)          |
| [docs/ROADMAP.md](docs/ROADMAP.md)           | Planned milestones and direction            |
| [BUILD.md](BUILD.md)                         | Environment setup and build instructions    |
| [CONTRIBUTING.md](CONTRIBUTING.md)           | Contribution workflow and standards         |
| [SECURITY.md](SECURITY.md)                   | Vulnerability disclosure                    |
| [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md)     | Community standards                         |

## Build at a glance

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./bin/zuu_json
```

For full environment setup, see [BUILD.md](BUILD.md).

## Benchmark

Performance measurements from `bin/zuu_json_benchmark` on a 20-core CPU @ 2.9 GHz with `min_time: 2.000` (best of N iterations, values in ns; throughput shown as Mi/s for raw byte rates and M/s / G/s for tokens or lookups).

```bash
2026-06-18T18:41:43+07:00
Running zuu_json_benchmark.exe
Run on (20 X 2918 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x10)
  L1 Instruction 32 KiB (x10)
  L2 Unified 1280 KiB (x10)
  L3 Unified 24576 KiB (x1)
--------------------------------------------------------------------------------------------------------
Benchmark                                              Time             CPU   Iterations UserCounters...
--------------------------------------------------------------------------------------------------------
Benchmark::SmallTokenizer/min_time:2.000          336623 ns       331434 ns         8533 bytes_per_second=930.066Mi/s
Benchmark::MediumTokenizer/min_time:2.000         415679 ns       403548 ns         6892 bytes_per_second=1.49316Gi/s
Benchmark::BigTokenizer/min_time:2.000           1046557 ns      1039720 ns         2675 bytes_per_second=1.59234Gi/s
Benchmark::HugeTokenizer/min_time:2.000         44401763 ns     42086694 ns           62 bytes_per_second=1.22935Gi/s
Benchmark::NumericTokenizer/min_time:2.000       1932048 ns      1930706 ns         1481 bytes_per_second=1.08585Gi/s
Benchmark::StringTokenizer/min_time:2.000         397221 ns       396729 ns         7168 bytes_per_second=1.51883Gi/s
Benchmark::ArrayTokenizer/min_time:2.000          319177 ns       317383 ns         8960 bytes_per_second=971.241Mi/s
Benchmark::ObjectTokenizer/min_time:2.000        1137154 ns      1119858 ns         2358 bytes_per_second=1.47839Gi/s
Benchmark::LiteralTokenizer/min_time:2.000           329 ns          324 ns      8533333 bytes_per_second=1.18105Gi/s
Benchmark::SmallParser/min_time:2.000             314560 ns       310407 ns         8960 Tokens/s=199.744M/s
Benchmark::MediumParser/min_time:2.000           3058099 ns      3032211 ns          943 Tokens/s=18.2256M/s
Benchmark::BigParser/min_time:2.000              2158827 ns      2097696 ns         1378 Tokens/s=64.8287M/s
Benchmark::HugeParser/min_time:2.000            50339716 ns     46622984 ns           62 Tokens/s=139.586M/s
Benchmark::NumericParser/min_time:2.000          2501966 ns      2392782 ns         1195 Tokens/s=139.743M/s
Benchmark::StringParser/min_time:2.000           3129988 ns      2999072 ns          943 Tokens/s=18.427M/s
Benchmark::ArrayParser/min_time:2.000             364920 ns       344413 ns         7168 Tokens/s=180.022M/s
Benchmark::ObjectParser/min_time:2.000           2176930 ns      2063679 ns         1378 Tokens/s=65.8974M/s
Benchmark::LiteralParser/min_time:2.000             92.9 ns         88.5 ns     33185185 Tokens/s=835.984M/s
Benchmark::SmallFullPipeline/min_time:2.000       695601 ns       647020 ns         4371 bytes_per_second=476.423Mi/s
Benchmark::MediumFullPipeline/min_time:2.000     3550375 ns      3393405 ns          815 bytes_per_second=181.83Mi/s
Benchmark::BigFullPipeline/min_time:2.000        3328354 ns      3069196 ns          896 bytes_per_second=552.366Mi/s
Benchmark::HugeFullPipeline/min_time:2.000      94826155 ns     90725806 ns           31 bytes_per_second=583.969Mi/s
Benchmark::NumericFullPipeline/min_time:2.000    4360941 ns      4331459 ns          689 bytes_per_second=495.625Mi/s
Benchmark::StringFullPipeline/min_time:2.000     3355534 ns      3355061 ns          815 bytes_per_second=183.909Mi/s
Benchmark::ArrayFullPipeline/min_time:2.000       665827 ns       666452 ns         4267 bytes_per_second=462.532Mi/s
Benchmark::ObjectFullPipeline/min_time:2.000     3200090 ns      3156390 ns          896 bytes_per_second=537.107Mi/s
Benchmark::LiteralFullPipeline/min_time:2.000        457 ns          458 ns      6179310 bytes_per_second=856.413Mi/s
Benchmark::DomTraversalArray/min_time:2.000        0.629 ns        0.624 ns   4480000000 Lookups/s=1.60179G/s
Benchmark::DomTraversalObject/min_time:2.000         135 ns          132 ns     20837209 Lookups/s=15.1543M/s
Benchmark::DomTraversalDeep/min_time:2.000          72.1 ns         69.8 ns     39822222 Lookups/s=57.2724M/s
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
