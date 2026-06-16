# Changelog

All notable changes to `zuu_json` are documented here. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and the project adheres to [Semantic Versioning](https://semver.org/).

## [Unreleased]

### Added
- **O(log N) object lookups** — `Json::operator[]`, `Value::operator[]`, and `Value::contains` now use `std::lower_bound` over sorted object members.
- **Sorted object members** — `Storage::sealObject` sorts members by key when `size > 1`, enabling binary search.
- **Arena pool (bump allocator)** — single contiguous byte buffer; all parser allocations are amortized.

### Changed
- Replaced O(N) linear scans in `Json::operator[]` and `Value::operator[]` / `Value::contains` with binary search.
- Refactored object member storage to a sorted layout; `sealObject` now performs an in-place `std::sort` over the just-sealed range.

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
