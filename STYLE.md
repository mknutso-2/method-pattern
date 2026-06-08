# Style

## C++

- Formatting is enforced by `clang-format` (see `.clang-format`).
- Naming is checked by `clang-tidy` (see `.clang-tidy`).

### C++ naming rules

- `PascalCase`: classes, structs, enums, namespaces, functions/methods, and static-storage `const`/`constexpr`
  variables.
- `snake_case`: variables, including local variables, parameters, struct fields, and non-static globals.
- `UPPER_SNAKE_CASE`: macros.
- `m_`: member instance variables of classes. Public struct fields use regular `snake_case`.
- `s_`: non-const static member variables.
- All C++ source files (`.h` and `.cpp`) use PascalCase filenames.
- Reusable class, struct, or enum declarations use one declaration per file. Files use `.h` and `.cpp`.
- The file name must match the class, struct, or enum name it declares.
- Do not declare public nested types. Public enums, classes, and structs must be declared directly in a namespace, not
  inside a class, so each type can be forward-declared independently.
- Public headers live under `include/<ProjectName>/` so consumer include paths are clearly project-scoped and less
  likely to collide with other libraries. Private headers live beside the implementation that uses them so they stay out
  of the supported API surface and remain easy to change.

### Includes

- Use angle brackets for standard library and external dependency headers, such as `<vector>`, `<catch2/...>`, and
  `<nlohmann/...>`.
- Use quotes for headers owned by this project, such as `"Method/Declaration.h"`.
- Keep include groups ordered as standard library headers first, external dependency headers next, and current-project
  headers last. Within each group, rely on `clang-format` to sort alphabetically.
- Public headers must be self-sufficient: each header should compile when included by itself, without relying on another
  header being included first. Run `bash scripts/ci-public-headers.sh` after changing public headers.

### Translation-unit local code

- Classes, structs, enums, and functions that are only used in one translation unit should usually be declared and
  defined in the `.cpp` file that uses them.
- Declare translation-unit local functions `static` instead of putting them in an anonymous namespace. This is not for
  different unused-function diagnostics: Clang warns about unused functions in either form when `-Wunused-function` is
  enabled. Prefer `static` because it keeps internal linkage visible at the function definition. This follows LLVM's
  rationale that anonymous namespace blocks reduce locality because readers must scan elsewhere in the file to know
  whether a function is local ([LLVM Coding Standards](https://llvm.org/docs/CodingStandards.html#restrict-visibility),
  [llvm-prefer-static-over-anonymous-namespace](https://clang.llvm.org/extra/clang-tidy/checks/llvm/prefer-static-over-anonymous-namespace.html)).
- Put translation-unit local classes, structs, and enums in an anonymous namespace because types cannot be marked
  `static`.
- Prefer these forms for small parsing helpers, test fixtures, local comparison functions, and other implementation
  details that do not need to be shared across source files.
- Declare translation-unit local helpers as late as possible, immediately before the first code that uses them. This
  applies to local classes, structs, enums, functions, constants, and variables. Avoid collecting all helper declarations
  at the top of a `.cpp` file unless ordering dependencies make that the clearest option.

### Namespace definitions

- In `.cpp` files, define reusable functions from named namespaces with a qualified name such as
  `Method::CreateDefinition(...)` instead of opening a `namespace Method { ... }` block around the definitions.
- Keep namespace context visible at each definition site. This follows the same readability concern behind LLVM's advice
  to keep namespace scopes small: large namespace blocks reduce locality because readers must scan around to know a
  function's scope ([LLVM Coding Standards](https://llvm.org/docs/CodingStandards.html#anonymous-namespaces)).
- Anonymous namespaces are still appropriate for translation-unit local types.

### Commenting

- Unless a declaration is totally trivial, every class/struct, class/struct member variable, enum, and enum value should
  have an associated comment.

### Running locally

- `clang-format -i <files>` uses `.clang-format` automatically.
- `clang-tidy -p <cmake-build-dir> <file>` works best with `compile_commands.json` when the active CMake generator
  emits it.
- `compile_flags.txt` gives clangd and standalone clang-tidy a fallback configuration when a compilation database is not
  available.

## CMake

- CMake formatting is enforced by `cmake-format` from `cmakelang` (see `.cmake-format.py`).
- Run `bash scripts/ci-cmake-format.sh` after changing `CMakeLists.txt` or `.cmake` files.
