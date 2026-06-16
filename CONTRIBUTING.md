# Contributing

Thanks for your interest in contributing to `zuu_json`. This document describes the workflow, conventions, and review expectations for the project.

## Branch strategy

| Branch | Purpose |
| --- | --- |
| `main` | Stable. Every commit here is release-quality and CI-green. |
| `ci/*` | Triggers CI for the matching push; mirror of `main` for pre-release verification. |
| `feat/*` | Feature work, branched from `main`. PRs target `main`. |
| `fix/*` | Bug fixes, branched from `main`. PRs target `main`. |
| `perf/*`, `refactor/*`, `chore/*` | Non-functional work, branched from `main`. PRs target `main`. |

Current development branch: `feat/DOM-improvement`.

## Pull request process

1. **Branch from `main`.** Branch names should follow the `type/short-topic` convention above.
2. **Keep commits focused.** One logical change per commit. Use the [Conventional Commits](https://www.conventionalcommits.org/) style:
   - `feat(parser): add comment-stripping fast path`
   - `fix(tokenizer): handle surrogate pair boundary`
   - `perf(storage): use std::lower_bound for object lookups`
   - `refactor(arena): pool chunks instead of one arena`
   - `chore: bump cmake_minimum to 3.21`
3. **Pass CI locally before opening the PR.** Run the same commands CI uses (see [BUILD.md](BUILD.md) → "CI build" section). Clang-Tidy and Valgrind must come back clean.
4. **Open the PR** against `main`. Fill in the description with:
   - What changed and why
   - Test plan / benchmark numbers when performance-related
   - Any follow-up TODOs left intentionally
5. **Address review feedback in new commits** when possible; squash before merge if the history is noisy.
6. **Squash-merge** is the default. The merge commit subject must follow Conventional Commits and reference the PR number.

## Coding standards

- **C++23, no extensions.** Set in `CMakeLists.txt` via `CMAKE_CXX_EXTENSIONS OFF`.
- **Naming.**
  - Types, concepts: `PascalCase`
  - Functions, variables: `snake_case`
  - Member fields: `snake_case_` (trailing underscore for non-static data members)
  - Enum members: `PascalCase`
  - Constants: `kPascalCase` or `UPPER_SNAKE` for `static constexpr` integer values
- **Headers.** Each public header has a Doxygen-style comment block with `@file`, `@brief`, `@author`, `@version`, `@date`, `@copyright`. Internal headers follow the same.
- **Error handling.** Use `std::expected<T, JsonError>` for fallible APIs. Do not introduce exceptions in library code.
- **`noexcept`.** Mark everything that cannot throw. The current code is `noexcept` by default; preserve that.
- **Allocations.** Avoid heap allocations in the hot path. The arena owns almost everything; respect that boundary.

## Formatting and linting

- **`clang-format`** is the source of truth. Run `cmake --build build --target format` before committing.
- **`clang-tidy`** runs in CI on every `.cpp` under `src/` and `internal/`. Use the project `.clang-tidy` config — do not override per-check in code.
- **Trailing newlines.** `value.cpp` is the only file that historically lacked one; please add it for any new file.

## Testing

There is no separate unit-test target today. Coverage is achieved through:

1. **The `zuu_json` demo binary** (`src/main.cpp`) which exercises the fluent API on a representative document.
2. **Google Benchmark** (`tests/benchmark.cpp`, built with `-DENABLE_BENMARK=ON`) which times the tokenizer, the parser, the full pipeline, and DOM traversal against sample fixtures in `samples/`.
3. **CI Valgrind pass** on Linux for leak and invalid-access detection.

If you add a new public API, extend `src/main.cpp` and/or `tests/benchmark.cpp` to cover it.

## Sample fixtures

Large JSON samples are pulled on demand by `scripts/fetch-sample.ps1`. They are vendored into `samples/` and excluded from GitHub language statistics via `.gitattributes` (`samples/** linguist-vendored`). Do not commit sample JSON by hand — let the script handle it so the provenance is clear.

## Performance work

Performance PRs must include:

- A short note on the technique (SWAR, lookup table, arena, sorted members, …)
- Before/after numbers from `zuu_json_benchmark` on at least `samples/twitter.json` and `samples/mesh.json`
- Confirmation that the `format-check` and `clang-tidy` targets are still green

## Communication

- Open an issue for design discussion before large refactors.
- Use PR comments for review feedback.
- Be respectful; this project follows the [Code of Conduct](CODE_OF_CONDUCT.md).
