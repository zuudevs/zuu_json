# API Reference

This document covers the **public** API of `zuu_json`. Anything under `internal/` is implementation detail and is not part of the stable surface.

## Headers

The public surface is concentrated in two headers, both reachable from the umbrella `zuu_json/json.hpp`:

| Header                                | Provides                          |
| ------------------------------------- | --------------------------------- |
| `zuu_json/json.hpp`                   | `zuu::Json`, `zuu::Value` aliases |
| `zuu_json/models/json.hpp`            | `zuu::models::Json`               |
| `zuu_json/models/value.hpp`           | `zuu::models::Value`              |
| `zuu_json/core/error.hpp`             | `zuu::core::JsonError`            |
| `zuu_json/utils/error_translator.hpp` | `zuu::utils::TranslateError`      |

The default include is `#include "zuu_json/json.hpp"`.

## Namespace layout

```text
zuu                    // user-facing aliases
└── models             // primary types
    ├── Json
    └── Value

zuu::core              // error enum
└── JsonError

zuu::utils             // free-function helpers
└── TranslateError
```

## Error type

`zuu::core::JsonError` is a strongly-typed enum that every fallible operation returns through `std::expected`.

| Member                  | When it is produced                                                         |
| ----------------------- | --------------------------------------------------------------------------- |
| `None`                  | Successful outcome.                                                         |
| `TrailingComma`         | A `,` appears where a value is expected.                                    |
| `SingleQuotedString`    | A `'` was used to delimit a string.                                         |
| `UnquotedKey`           | An object key is missing `"` quotes.                                        |
| `MissingComma`          | Two values are adjacent without a separator.                                |
| `UnescapedCharacter`    | An invalid escape sequence appears in a string.                             |
| `InvalidValue`          | A literal could not be matched; **also** returned for a missing object key. |
| `CommentNotAllowed`     | `//` or `/* */` appears in strict mode.                                     |
| `InvalidBooleanLiteral` | Token is not exactly `true` or `false`.                                     |
| `InvalidNullLiteral`    | Token is not exactly `null`.                                                |
| `EmptyValue`            | A position where a value is required is empty.                              |
| `LeadingZero`           | A numeric literal starts with a `0` other than `0` itself.                  |
| `InvalidType`           | A typed access (`as_*`) was called on the wrong type.                       |
| `IsNotArray`            | An array operation was applied to a non-array.                              |
| `IsNotObject`           | An object operation was applied to a non-object.                            |
| `RootNotArrayType`      | Array root was expected.                                                    |
| `RootNotObjectType`     | Object root was expected.                                                   |
| `InvalidUnicode`        | A `\uXXXX` sequence is not a valid code point.                              |
| `InvalidSurrogate`      | A surrogate pair is malformed.                                              |
| `Unknown`               | Catch-all for unrecoverable states.                                         |

`zuu::utils::TranslateError(JsonError)` returns a human-readable `const char*` for the enum.

## `zuu::Json`

A fully-parsed JSON document. Move-only and non-copyable. The `Json` object owns its `Storage` via `std::unique_ptr`; `Value` views into the document remain valid for the lifetime of the `Json` that produced them.

### Construction

`Json` cannot be constructed directly by user code. The only entry point is the static factory:

```cpp
[[nodiscard]] static Result<Json> Json::parse(std::string_view content) noexcept;
```

| Parameter | Description                                                                                            |
| --------- | ------------------------------------------------------------------------------------------------------ |
| `content` | The full JSON text to parse. The view is not retained; the parser copies what it needs into the arena. |

| Return         | Description                                                 |
| -------------- | ----------------------------------------------------------- |
| `Result<Json>` | `Json` on success; `std::unexpected{JsonError}` on failure. |

### Lifetime

```cpp
auto doc = zuu::Json::parse(text);   // Result<Json>
// doc is a std::expected<Json, JsonError>
if (doc) {
    auto root = doc->root();
    // ... use root ...
}   // doc destructor frees Storage
```

### Accessors

```cpp
[[nodiscard]] Value root() const noexcept;
```

Returns a `Value` view of the document's root. The view is valid for the lifetime of the `Json` object.

```cpp
[[nodiscard]] Result<Value> operator[](std::string_view key) const noexcept;
```

Looks up `key` in the root object using **O(log N) binary search** (object members are sorted by key when the object is sealed). Returns `IsNotObject` if the root is not an object, or `InvalidValue` if the key is missing.

## `zuu::Value`

A read-only view of a single JSON value. Holds a pointer to the owning `Storage` and a 64-bit `JsonValue` (type tag + payload). The view is cheap to copy and does not own any resources.

### Type predicates

| Method         | Returns `true` when…                                                                    |
| -------------- | --------------------------------------------------------------------------------------- |
| `is_null()`    | Type is `Null`.                                                                         |
| `is_bool()`    | Type is `Boolean`.                                                                      |
| `is_integer()` | Type is `Integer`.                                                                      |
| `is_double()`  | Type is `Double` (the bit pattern of a normal double is preserved in the storage word). |
| `is_string()`  | Type is `String`.                                                                       |
| `is_array()`   | Type is `Array`.                                                                        |
| `is_object()`  | Type is `Object`.                                                                       |

```cpp
[[nodiscard]] Type type() const noexcept;
```

Returns the underlying `Type` enum value.

### Strict typed access

These return `Result<T>` and signal `JsonError::InvalidType` on a type mismatch.

```cpp
[[nodiscard]] Result<bool>            as_bool()    const noexcept;
[[nodiscard]] Result<long long>       as_integer() const noexcept;
[[nodiscard]] Result<long double>     as_double()  const noexcept;
[[nodiscard]] Result<std::string_view> as_string() const noexcept;
```

```cpp
auto v = doc["n"].as_integer();
if (v) {
    use(*v);
} else {
    handle(v.error());
}
```

### Fluent typed access (with default)

These return a value of the requested type, falling back to a default on type mismatch or missing key. They never return an error.

```cpp
[[nodiscard]] bool            get_bool(bool default_val = false) const noexcept;
[[nodiscard]] long long       get_integer(long long default_val = 0) const noexcept;
[[nodiscard]] double          get_double(double default_val = 0.0) const noexcept;
[[nodiscard]] std::string_view get_string(std::string_view default_val = "") const noexcept;
```

```cpp
std::println("{}", doc["missing"].get_string("fallback"));
```

### Container inspection

```cpp
[[nodiscard]] unsigned long long size() const noexcept;
```

Returns the number of elements (for arrays) or members (for objects). Returns `0` for all other types.

```cpp
[[nodiscard]] bool contains(std::string_view key) const noexcept;
```

Returns `true` if the value is an object that contains `key`. Implemented with O(log N) binary search.

### Strict traversal

```cpp
[[nodiscard]] Result<Value> at(unsigned long long index) const noexcept;
[[nodiscard]] Result<Value> at(std::string_view key) const noexcept;
```

- `at(unsigned long long)` — bounds-checked array access. Returns `InvalidValue` on out-of-range.
- `at(string_view)` — object lookup. Returns `IsNotObject` or `InvalidValue` as appropriate.

### Fluent traversal (optional chaining)

```cpp
[[nodiscard]] Value operator[](unsigned long long index) const noexcept;
[[nodiscard]] Value operator[](std::string_view key) const noexcept;
```

These do **not** error. A missing key, out-of-range index, or wrong container type returns a `Value` of type `Null` — making chained lookups safe.

```cpp
// Safe: no exception, no unwrap, no crash on missing keys
std::println("{}", root["user"]["address"]["city"].get_string("unknown"));
```

For objects, the lookup uses O(log N) binary search.

## Errors as `std::expected`

Every fallible call returns `std::expected<T, JsonError>`. The library never throws. Example handling pattern:

```cpp
auto parsed = zuu::Json::parse(text);
if (!parsed) {
    std::println("error: {}", zuu::utils::TranslateError(parsed.error()));
    return 1;
}
```

## End-to-end example

```cpp
#include "zuu_json/json.hpp"
#include "zuu_json/utils/error_translator.hpp"
#include <print>

int main() {
    auto doc = zuu::Json::parse(R"({
        "user": { "name": "zuu", "age": 21 },
        "tags": ["c++", "json", "perf"]
    })");

    if (!doc) {
        std::println("parse error: {}", zuu::utils::TranslateError(doc.error()));
        return 1;
    }

    const auto root = doc->root();

    // Strict path
    if (auto name = root["user"]["name"].as_string(); name) {
        std::println("name = {}", *name);
    }

    // Fluent path with default
    std::println("age  = {}", root["user"]["age"].get_integer(-1));

    // Iteration via size()
    auto tags = root["tags"];
    for (unsigned long long i = 0; i < tags.size(); ++i) {
        std::println("tag[{}] = {}", i, tags[i].get_string());
    }

    // contains()
    if (root["user"].contains("email")) {
        std::println("email present");
    }
}
```
