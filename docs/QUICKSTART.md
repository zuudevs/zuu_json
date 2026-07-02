# Quickstart

This guide gets you from a fresh clone to a running `zuu_json` program in a few minutes. For deeper information, see [API.md](API.md), [ARCHITECTURE.md](ARCHITECTURE.md), and [../BUILD.md](../BUILD.md).

## 1. Prerequisites

You need:

- A C++23 compiler (Clang 17+, GCC 13+, or MSVC 17.8+)
- CMake 3.20 or newer
- Ninja (recommended) or another CMake generator

No other libraries are required for the core build.

## 2. Clone

```bash
git clone https://github.com/zuudevs/zuu_json.git
cd zuu_json
```

## 3. Configure and build

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The resulting binary is `./bin/zuu_json` (or `bin\zuu_json.exe` on Windows).

## 4. Run the demo

```bash
./bin/zuu_json
```

Expected output:

```text
--- FLUENT API SHOWCASE ---
name    : zuu
active  : true
age     : 21
pi      : 3.14
pos.x   : 20.5
pos.y   : 10.5
data    : is explicitly null
missing : default_string
colors  : 2 item(s)
  [0]   : white
  [1]   : blue
```

The demo lives in [`src/main.cpp`](../src/main.cpp) and exercises every public API surface.

## 5. Your first program

Create a file `hello.cpp`:

```cpp
#include "zuu_json/json.hpp"
#include <print>

int main() {
    auto doc = zuu::Json::parse(R"({"hello": "world", "n": 42})");
    if (!doc) {
        std::println("parse failed: {}", static_cast<int>(doc.error()));
        return 1;
    }

    const auto root = doc->root();
    std::println("hello = {}", root["hello"].get_string());
    std::println("n     = {}", root["n"].get_integer());
}
```

Build it against the project library:

```bash
g++ -std=c++23 -O3 hello.cpp -Iinclude -Llib -lcpp_json_lib -o hello
```

(Adjust `libcpp_json_lib` to `cpp_json_lib` on Windows / MSVC.)

## 6. Link into a CMake project

If you build `zuu_json` as a subdirectory of your project, link against the `cpp_json_lib` target and add `include/` to your include path:

```cmake
add_subdirectory(zuu_json)
target_link_libraries(your_app PRIVATE cpp_json_lib)
```

## 7. Next steps

- Read [API.md](API.md) for the full public surface.
- Read [ARCHITECTURE.md](ARCHITECTURE.md) to understand the tokenizer → parser → storage pipeline.

## Running the benchmark suite

Performance work uses the per-domain benchmark files in `tests/`. Enable the benchmark target at configure time:

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_BENCHMARK=ON
cmake --build build --target zuu_json_benchmark
```

The benchmark binary pulls in sample fixtures from `samples/` via `internal/utils/fs_util.hpp` — the loader walks up to four parent directories from the working directory, so it works no matter where you launch it from.

Filter by benchmark name with `--benchmark_filter`. Example: only the DOM and end-to-end pipeline runs:

```bash
./bin/zuu_json_benchmark --benchmark_filter='DOM_|Pipeline_'
```

See [ARCHITECTURE.md](ARCHITECTURE.md#benchmark-layout) for a list of the per-domain files.
