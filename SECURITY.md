# Security Policy

## Supported versions

| Version | Supported |
| --- | --- |
| `1.x` (current) | ✅ Active development; security fixes backported |
| `< 1.0` | ❌ No longer supported |

The library is currently at version `1.0` (see `CMakeLists.txt`). Pre-1.0 development tags are not maintained.

## Reporting a vulnerability

Please **do not file a public issue** for suspected security vulnerabilities.

Instead, report privately:

- **Email:** zuudevs@gmail.com
- **Subject prefix:** `[zuu_json security]`

Include:

1. A description of the vulnerability and its impact (DoS, info disclosure, memory corruption, …).
2. A minimal reproducer — a JSON input, the code path exercised, and observed vs. expected behavior.
3. The commit hash or release tag you tested against.
4. Your contact information and preferred disclosure timeline.

You should receive an acknowledgement within **72 hours**. We aim to provide a remediation plan within **7 days** of confirmation.

## Disclosure process

1. **Triage** — the maintainers confirm the report and assign a CVSS-style severity.
2. **Patch** — a fix is developed in a private branch. CI runs privately if needed.
3. **Coordinated disclosure** — we publish the patch and a release together with a short advisory. Reporters are credited by name in the advisory unless they request otherwise.
4. **Post-mortem** — for significant issues, a brief write-up of root cause and mitigation is added to [docs/CHANGELOG.md](docs/CHANGELOG.md).

## Security commitments

- The library is **exception-free** in its public API. Fallible operations return `std::expected<T, JsonError>`, so callers cannot crash via uncaught exceptions on malformed input.
- `Storage` is non-copyable and lives behind `std::unique_ptr` in `Json`. Use-after-free of a parsed document's internal state requires a use-after-free of the `Json` object itself, which is the caller's responsibility.
- The parser rejects single-quoted strings, unquoted keys, comments, leading zeros, invalid unicode and surrogate pairs, and many other shapes that have historically produced parser differentials. See [`core/error.hpp`](include/zuu_json/core/error.hpp) for the full list of `JsonError` values.
- CI runs the `zuu_json` binary under Valgrind with `--leak-check=full` on every push to `main` and on every PR.

## Out of scope

- Issues in third-party dependencies pulled in by `ENABLE_BENCHMARK=ON` (Google Benchmark). Report those upstream.
- Theoretical concerns without a concrete reproducer.

## Acknowledgements

We are grateful to the security community. Reporters who follow this policy are credited in release notes unless they opt out.
