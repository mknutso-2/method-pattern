# Method Pattern

[![CI](https://github.com/mknutso-2/method-pattern/actions/workflows/ci.yml/badge.svg)](https://github.com/mknutso-2/method-pattern/actions/workflows/ci.yml)

## Overview

This repo contains a small C++23 Method Pattern library. It is similar to the Command pattern, but uses "Method" because
the model supports both read-oriented queries and action-oriented commands.

The main idea is to document and implement a complete interaction surface for a library or application as a list of
Methods. In this README, that list is called a **Method Surface**. "Control Surface" is also a reasonable phrase, but
"Method Surface" is more precise here because the surface includes queries as well as commands, and because the term
maps directly to a collection of `Method::Declaration` and `Method::Definition` objects.

The public CMake target is `Method::Method`, and public headers live under `include/Method/`.

## Method Surface

A Method Surface is the complete set of operations a library or application chooses to expose. Each operation is
described by a `Method::Declaration` and implemented by a `Method::Definition`.

For a library, the Method Surface can define the externally visible automation contract for querying or changing the
library's state. A consuming application, plugin host, service process, or future automation protocol can enumerate the
surface and decide which methods to expose.

For an application, the Method Surface usually sits directly on top of the model layer in a Model-View architecture. The
view layer invokes Methods instead of reaching into the model through scattered one-off calls. If all meaningful
model-view interactions go through Methods, the application has a concise, inspectable definition of its interaction
surface.

This library intentionally does not define an automation protocol. It defines the in-process model that a protocol could
use: declarations, definitions, invocations, schemas, operation kind, versioning, descriptions, and undo behavior.

## Core Types

- `Method::Declaration` stores stable metadata: ID, display name, description, kind, version, input JSON schema, and
  output JSON schema.
- `Method::Definition` stores a declaration and an invocation factory.
- `Method::Invocation` stores the declaration and per-invocation description, requires `Execute()`, and provides a
  default `Undo()` implementation that throws.
- `Method::Kind` distinguishes `Query`, `UndoableCommand`, and `NonUndoableCommand`.
- `Method::Version` stores major and minor versions.
- `Method::Json` aliases `nlohmann::json` for schema values.

## Why Use It

Representing an interaction surface as Methods has several practical benefits:

- Discoverability: applications can enumerate available Methods for AI interaction, generated documentation, command
  palettes, user-mappable shortcuts, plugin hosts, or external automation tools.
- Auditing and telemetry: applications can consistently log, record, measure, and replay all operations performed during
  a session when model interaction flows through Method invocations.
- Portability across views: the same Method Surface can back a desktop UI, CLI, web UI, test harness, scripting bridge,
  or remote automation layer without redefining the model-facing API each time.
- Automation readiness: once an application has a complete Method Surface with schemas, descriptions, and operation
  kinds, exposing that surface through an automation protocol becomes a direct adapter problem.
- Undo structure: `Method::Kind::UndoableCommand` gives the API an explicit, opinionated place to introduce undo
  behavior where it is useful.
- Validation and tooling: input and output schemas give callers, adapters, tests, and documentation generators a common
  description of expected payloads.
- Permission boundaries: declarations provide a natural place for future policy metadata, allowing hosts to decide which
  Methods are visible or callable in a given context.
- Testing: Methods make integration tests and model-level workflow tests easier to express because tests can invoke the
  same surface used by the UI or automation layer.
- Versioning: declarations include a version so callers can reason about compatibility and evolution at the operation
  level, not only at the library or application level.

## Typical Application Shape

In an application, Method often fits as a thin layer above the model:

```text
View / UI / CLI / Automation Adapter
   -> Method::Definition lookup
   -> Method::Invocation
   -> Model layer
```

The view chooses a Method, supplies input, creates an invocation, and executes it. The model still owns the domain logic;
Method provides a consistent way to name, describe, invoke, audit, and optionally undo that logic.

## Typical Library Shape

A library can expose a function that returns its Method declarations or definitions:

```cpp
std::vector<Method::Declaration> ListWidgetMethods();
std::vector<Method::Definition> CreateWidgetMethods(WidgetModel & model);
```

That list can then be consumed by application code, documentation generators, AI tooling, or an automation adapter. The
transport is deliberately outside this library; Method is the shared vocabulary for describing and invoking the
operations.

## Examples

The repo builds examples by default when Method is the top-level CMake project. Consumers that bring Method in through
`FetchContent` do not build examples unless they set `METHOD_BUILD_EXAMPLES=ON`.

- `CounterExample` is a compact model-plus-methods sample. It shows a query, an undoable command, and a non-undoable
  command against a tiny counter model.
- `TodoListCliExample` and `TodoListAutomationExample` share the same TODO list Method Surface. The TODO example is
  split into a core model library, a methods library, and two surfaces. The CLI and automation executables include only
  the methods-layer public header, so the model remains behind the Method Surface.

The automation example accepts either a single JSON request or an array of requests. Each request has a `method` field
and optional `parameters` object:

```json
[
   { "method": "todo.add", "parameters": { "title": "Write docs" } },
   { "method": "todo.complete", "parameters": { "id": 1 } },
   { "method": "todo.list", "parameters": {} }
]
```

## Project Layout

- `include/Method/` contains public library `.h` headers.
- `src/` contains `.cpp` implementation files.
- `examples/` contains the Counter and TODO list examples.
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
