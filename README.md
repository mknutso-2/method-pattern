# Method Pattern

[![CI](https://github.com/mknutso-2/method-pattern/actions/workflows/ci.yml/badge.svg)](https://github.com/mknutso-2/method-pattern/actions/workflows/ci.yml)

## Overview

This repo contains a small C++23 Method Pattern library. It is similar to the Command pattern, but uses "Method" because
the model supports both read-oriented queries and action-oriented commands.

The public CMake target is `Method::Method`, and public headers live under `include/Method/`.

## Core Types

- `Method::Declaration` stores stable metadata: ID, display name, description, kind, version, input JSON schema, and
  output JSON schema.
- `Method::Definition` stores a declaration and an invocation factory.
- `Method::Invocation` stores the declaration and per-invocation description, requires `Execute()`, and provides a
  default `Undo()` implementation that throws.
- `Method::Kind` distinguishes `Query`, `UndoableCommand`, and `NonUndoableCommand`.
- `Method::Version` stores major and minor versions.
- `Method::Json` aliases `nlohmann::json` for schema values.

## Project Layout

- `include/Method/` contains public library `.h` headers.
- `src/` contains `.cpp` implementation files.
- `tests/` contains Catch2 unit tests.
- `STYLE.md` documents the local C++ style rules used by this library.
- `.sh` files in `scripts/` define build, linting, and testing commands that are shared between GitHub Actions, local
  command-line usage, and Visual Studio Code tasks.
- `.vscode/` contains IDE configuration, debugger launch settings, suggestions, and task wiring.
- `.editorconfig` enforces cross-editor formatting defaults.
- `.gitattributes` pins line endings to LF and marks binary assets so Git does not normalize them.
- `.cmake-format.py` configures CMake file formatting.

## Requirements

Required:

- CMake 3.24 or newer.
- A C++23 compiler and a matching CMake generator.
- `bash` on `PATH` to run the scripts in `scripts/` and the provided Visual Studio Code tasks.
- `cmake-format` from `cmakelang` to run `bash scripts/ci-cmake-format.sh`.
- `clang-format` and `clang-tidy` on `PATH` to run `bash scripts/ci-lint.sh`.
- Network access during first configure unless Catch2 and `nlohmann/json` are already available through the active
  toolchain.

Supported compiler families:

- MSVC on Windows.
- Clang.
- GCC.

The same warning policy applies to all supported compiler families: use the highest practical project warning level,
treat warnings as errors, and suppress only documented low-signal diagnostics. The exact compiler flags differ because
MSVC, Clang, and GCC do not expose the same warning switches.

Optional:

- Ninja. The default MSVC scripts do not require Ninja, but the GCC and Clang presets use Ninja so they can work
  consistently on Windows and Linux when those toolchains are installed.
- Visual Studio Code with the recommended extensions in `.vscode/extensions.json`.

## Using With FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
   MethodPattern
   GIT_REPOSITORY https://github.com/mknutso-2/method-pattern.git
   GIT_TAG <commit-or-release-tag>
)
FetchContent_MakeAvailable(MethodPattern)

target_link_libraries(MyTarget PRIVATE Method::Method)
```

Tests are enabled when this project is built as the top-level CMake project. When consumed through `FetchContent`, tests
are disabled by default. Set `METHOD_BUILD_TESTING=ON` to force Method's tests on in a consuming build.

## Installing For find_package

This repo can install and export a CMake package when `nlohmann_json::nlohmann_json` is available through
`find_package(nlohmann_json CONFIG)`. Configure with `-DMETHOD_ENABLE_INSTALL=ON`, then install the project. Consumers
can then use:

```cmake
find_package(Method CONFIG REQUIRED)
target_link_libraries(MyTarget PRIVATE Method::Method)
```

## Quick Start

1. Build the default Debug preset:
   - `bash scripts/ci-build.sh`
2. Run tests:
   - `bash scripts/ci-unit-tests.sh`
3. Run a specific CI job locally when needed:
   - `bash scripts/ci-cmake-format.sh`
   - `bash scripts/ci-public-headers.sh`
   - `bash scripts/ci-build-clang.sh`
   - `bash scripts/ci-unit-tests-gcc-release.sh`
   - `bash scripts/ci-sanitizers-clang.sh`

## CI Reproducibility

Every GitHub Actions check job calls a matching script under `scripts/`, and every job script has a matching Visual
Studio Code task. This is intentional: a GitHub failure should be reproducible locally by running the same script or the
same VS Code task, and a local failure should indicate which CI job would fail.

`ci-build.sh` and `ci-unit-tests.sh` are lower-level helpers for default or custom preset usage. `ci-all.sh` runs the
checks that should work on the current platform and treats failures in those checks as errors. It also tries checks that
may be available through optional local toolchains, such as GCC or Clang with Ninja on Windows; missing or failing
optional checks are reported as warnings.

## Quality Baseline

Recommended defaults while working in this repo:

- Follow `STYLE.md` for naming, comments, and one-type-per-file organization.
- The project intentionally treats compiler warnings as errors. CMake applies the same strict warning policy to the
  library and test targets. Third-party dependency targets are not forced into this warning policy.
- Enabled `clang-tidy` findings are also treated as errors.
- Run `bash scripts/ci-cmake-format.sh` after changing CMake files.
- Run `bash scripts/ci-lint.sh` before merging formatting or lint-sensitive changes.
- Run `bash scripts/ci-public-headers.sh` after changing public headers.
- Run the matching `scripts/ci-*.sh` job script before merging compiler- or platform-sensitive changes.
- Run Release build/test scripts when a change could depend on optimizer behavior or `NDEBUG`.
- Run `bash scripts/ci-all.sh` before merging broad changes.
- Run `bash scripts/ci-sanitizers-clang.sh` to run the Clang AddressSanitizer and UndefinedBehaviorSanitizer build
  locally on platforms where Clang sanitizers are available.
