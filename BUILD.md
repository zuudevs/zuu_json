# Build

This document covers how to build `zuu_json` from source. For installing and first-run usage, see [docs/QUICKSTART.md](docs/QUICKSTART.md).

## Requirements

| Tool | Minimum version | Notes |
| --- | --- | --- |
| CMake | 3.20 | Required by `CMakeLists.txt` |
| C++ compiler | C++23 support | Clang 17+, GCC 13+, or MSVC 17.8+ |
| Ninja | 1.10+ | Recommended generator; enables `compile_commands.json` on Windows |
| PowerShell | 7+ | Only required for helper scripts in `scripts/` |

The library core has **no third-party runtime dependencies**. Google Benchmark is the only optional third-party component and is only pulled in when `-DENABLE_BENCHMARK=ON` is set.

## Standard library requirements

`zuu_json` relies on the following C++23 standard library features:

- `<expected>` — `std::expected<T, E>` for fallible APIs
- `<span>` — non-owning view types for arrays and objects
- `<bit>` — `std::bit_cast` for type punning
- `<print>` — used only by the demo `main.cpp`; not required by the library
- `<algorithm>` — `std::sort`, `std::lower_bound`

## Compiler support

| Compiler | Tested via |
| --- | --- |
| Clang | Linux CI, local development |
| GCC | Linux CI |
| MSVC | Windows local builds |

Release-mode optimizations are configured per-compiler inside `enable_release_optimizations` in `CMakeLists.txt`:

- **Clang / GCC:** `-O3 -march=native -mtune=native -DNDEBUG -ffunction-sections -fdata-sections`, linker GC sections, LTO via IPO when supported.
- **MSVC:** `/O2 /GL /DNDEBUG`, `/LTCG` for link-time code generation.

## Configure and build

### Linux / macOS

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The binary lands in `./bin/zuu_json`.

### Windows (PowerShell)

```pwsh
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
.\bin\zuu_json.exe
```

### Using the helper script

A PowerShell wrapper handles configure, build, run, and clean for the `zuu_json` and `zuu_json_benchmark` targets:

```pwsh
./scripts/dev.ps1 -opt configure -mode Release
./scripts/dev.ps1 -opt build
./scripts/dev.ps1 -opt run
./scripts/dev.ps1 -opt clean
```

To build the benchmark target:

```pwsh
./scripts/dev.ps1 -opt configure -mode Release -target zuu_json_benchmark
./scripts/dev.ps1 -opt build   -target zuu_json_benchmark
./scripts/dev.ps1 -opt run     -target zuu_json_benchmark
```

## Build options

| Option | Default | Effect |
| --- | --- | --- |
| `CMAKE_BUILD_TYPE` | `Release` | Release/RelWithDebInfo/Debug |
| `ENABLE_BENCHMARK` | `OFF` | Pulls in `third_party/google-benchmark` and builds the `zuu_json_benchmark` target |

Example with benchmarks:

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_BENCHMARK=ON
cmake --build build
```

## CI build

GitHub Actions builds on both `ubuntu-24.04` and `windows-latest` runners. See [`.github/ci-pipeline.yml`](.github/ci-pipeline.yml) for the full pipeline. Notable points:

- Builds run in **Debug** to keep clang-tidy and Valgrind outputs meaningful.
- Clang-Tidy is applied to every `.cpp` file under `src/` and `internal/`.
- Valgrind runs the `zuu_json` binary on Linux to catch leaks and invalid accesses.
- Sample fixtures are pulled by `scripts/fetch-sample.ps1` before configure.

To reproduce the CI build locally:

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --config Debug --parallel 4
find src internal -name "*.cpp" | xargs clang-tidy -p build/
```

## Code style tooling

```bash
# Format all sources in place
cmake --build build --target format

# Verify formatting without changes
cmake --build build --target format-check
```

`clang-tidy` is wired into the `cpp_json_lib` and `zuu_json` targets when `clang-tidy` is on `PATH`.

## Output layout

| Path | Contents |
| --- | --- |
| `bin/` | Runtime executables (`zuu_json`, `zuu_json_benchmark` when enabled) |
| `lib/` | Static library archive (`libcpp_json_lib.a` / `cpp_json_lib.lib`) |
| `build/` | Intermediate object files, CMake configuration, `compile_commands.json` |
