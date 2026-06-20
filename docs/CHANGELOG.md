# Changelog

All notable changes to `zuu_json` are documented here. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and the project adheres to [Semantic Versioning](https://semver.org/).

## [Unreleased]

### Added
- **O(log N) object lookups** — `Json::operator[]`, `Value::operator[]`, and `Value::contains` now use `std::lower_bound` over sorted object members.
- **Sorted object members** — `Storage::sealObject` sorts members by key when `size > 1`, enabling binary search.
- **Arena pool (bump allocator)** — single contiguous byte buffer; all parser allocations are amortized.
- **Cache-line aligned arena allocation** — `Storage` over-allocates by `cache_line_size - 1` bytes so the sub-region pointers can be aligned regardless of the `std::byte[]` base address.
- **Per-domain benchmark files** — the single `tests/benchmark.cpp` was split into `bm_main.cpp` (Google Benchmark entry point) plus one file per concern: `bm_tokenizer`, `bm_parser`, `bm_pipeline`, `bm_storage`, `bm_dom`, `bm_strings`, `bm_number`, `bm_error_path`.
- **`internal/utils/fs_util.hpp`** — new internal header that exposes `zuu::tests::utils::get_sample_path()` and `load_sample()`. Replaces the sample-loading helpers that previously lived on the old `Benchmark` helper class.

### Changed
- Replaced O(N) linear scans in `Json::operator[]` and `Value::operator[]` / `Value::contains` with binary search.
- Refactored object member storage to a sorted layout; `sealObject` now performs an in-place `std::sort` over the just-sealed range.
- Benchmark target (`${PROJECT_NAME}_benchmark`) now compiles the per-domain `bm_*.cpp` files. The benchmark include path now also adds `${CMAKE_SOURCE_DIR}/tests` so files can include `"utils/fs_util.hpp"`.
- Release link on MSVC switched from `/OPT:REF` to `/OPT:NOREF` to keep arena-aligned symbols alive (the address arithmetic in `Storage` references the arena base pointer and would otherwise be stripped by `/OPT:REF`).

## [1.0.0] - 2026-06-05

### Added
- Initial public release.
- `zuu::Json::parse` — entry point that returns `std::expected<Json, JsonError>`.
- `zuu::Value` — non-owning, copyable view with type predicates, strict and fluent accessors, and optional-chaining `operator[]`.
- Tokenizer with 256-byte lookup table for branch-free dispatch.
- SWAR (SIMD-Within-A-Register) whitespace scanning.
- Type-tagged 64-bit `JsonValue` (NaN-boxed encoding for non-double types).
- Static `Storage` arena with hint-driven pre-reservation.
- `JsonError` enum and `zuu::utils::TranslateError` for human-readable messages.
- `clang-format` and `clang-tidy` integration; `format` and `format-check` targets.
- Cross-platform CI on `ubuntu-24.04` and `windows-latest`; Valgrind on Linux.

### Notes
- The library is exception-free on the public API; the C++ standard library is allowed to throw internally and the library never unwinds.
- Doxygen-style header comments throughout; the public surface is small (`Json`, `Value`, `JsonError`, `TranslateError`).

[Unreleased]: https://github.com/zuudevs/zuu_json/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/zuudevs/zuu_json/releases/tag/v1.0.0
