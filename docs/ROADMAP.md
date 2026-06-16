# Roadmap

This document describes the planned direction of `zuu_json`. Items are grouped by horizon and may shift as priorities change.

## Near-term (next minor releases)

### JSON Pointer (`RFC 6901`)
- Add a `Value::at_json_pointer(std::string_view)` traversal API.
- Accept both `~0` / `~1` escapes and `-` (array end) for the array case.

### JSON Patch (`RFC 6902`)
- Apply and produce patches; integrate with `JsonPointer` for addressing.

### Streaming parser
- An incremental, pull-based parser that emits `Token` callbacks.
- Useful for `> 100 MB` documents where materializing the full DOM is not desirable.

### Schema validation
- A schema language companion (subset of JSON Schema) to validate parsed documents.

## Medium-term

### SAX-style DOM events
- An `EventSink` interface that gets called for each `Token` and each container open/close.
- Lets users build their own projections (e.g., flatten, summarize) without paying for the DOM.

### UTF-8 normalization
- Optional NFC normalization for object keys at parse time. Disabled by default; behind a compile-time switch.

### SIMD JSON syntax check (Stage 1)
- A pre-tokenization pass that uses SSE2/NEON byte scans to reject malformed JSON in big chunks before the byte-level tokenizer runs.

### Build system
- Move to a header-only distribution option for the most common configurations.
- Add a `find_package(zuu_json)` config-file template so consumers can pick the library up via `FetchContent` or system install.

## Long-term / strategic

### WASM target
- A `zuu_json_wasm` build configuration with a smaller surface for browser embedding.
- Keep the C++23 standard library requirement: rely on Emscripten's libc++ build.

### Conformance suite
- Integrate the [JSONTestSuite](https://github.com/nst/JSONTestSuite) y_* / n_* / i_* test corpus into CI and report coverage.

### Multi-language bindings
- A stable C ABI layer (the lowest common denominator) to enable C, Python (cffi), and other language bindings without restating the C++ API.

### Specification tracker
- Track every deviation from the [RFC 8259](https://www.rfc-editor.org/rfc/rfc8259) grammar in this document. Current known choices:
  - Numbers that exceed 48-bit integer range are demoted to `double`.
  - Comments and single-quoted strings are rejected, not accepted.
  - Duplicate object keys are accepted; the last one wins on lookup.

## Non-goals

- **JSON5 / HJSON / NDJSON.** Out of scope for the core library; if needed, they should live in a separate adapter.
- **Mutation APIs.** The library is read-only by design; mutation introduces lifetime and rebalancing concerns that conflict with the arena model. A `Builder` is a possible future addition.
- **A UI / IDE plugin ecosystem.** The maintainers will not ship editor tooling.

## How to influence this list

Open an issue tagged `roadmap` with a short proposal. Larger items (JSON Patch, streaming) should be preceded by a design note that covers the API shape and the cost model.
