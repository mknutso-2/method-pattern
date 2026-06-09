#!/usr/bin/env bash
# Mirrors the GitHub Actions ci.yml "lint" job.
# See also: ci-unit-tests.sh and ci-all.sh.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

if [[ -z "${CMAKE_PRESET:-}" && -z "${BUILD_DIR:-}" ]]; then
   CMAKE_PRESET="$(DefaultCMakePreset)"
fi

BUILD_TYPE="${BUILD_TYPE:-Debug}"
CLANG_FORMAT="${CLANG_FORMAT:-clang-format}"
CLANG_TIDY="${CLANG_TIDY:-clang-tidy}"
CLANG_TIDY_USE_COMPILE_COMMANDS="${CLANG_TIDY_USE_COMPILE_COMMANDS:-false}"

RunClangTidy() {
   set +e
   local output
   output="$("$CLANG_TIDY" --quiet "$@" 2>&1)"
   local status=$?
   set -e

   if [[ -n "$output" ]]; then
      printf "%s\n" "$output" | grep -Ev '^[0-9]+ warnings generated\.$' || true
   fi
   return "$status"
}

if [[ -n "${CMAKE_PRESET:-}" ]]; then
   BUILD_DIR="${BUILD_DIR:-build/$CMAKE_PRESET}"

   echo "==> cmake configure ($CMAKE_PRESET)"
   cmake --preset "$CMAKE_PRESET"
else
   BUILD_DIR="${BUILD_DIR:-build}"

   echo "==> cmake configure"
   cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DBUILD_TESTING=ON
fi

mapfile -t SOURCES < <(find examples include src tests -type f \( -name '*.h' -o -name '*.cpp' \) | sort)
mapfile -t TIDY_SOURCES < <(find examples src -type f -name '*.cpp' | sort)

echo "==> clang-format"
"$CLANG_FORMAT" --dry-run --Werror "${SOURCES[@]}"

echo "==> clang-tidy"
if [[ "$CLANG_TIDY_USE_COMPILE_COMMANDS" == "true" && -f "$BUILD_DIR/compile_commands.json" ]]; then
   RunClangTidy -p "$BUILD_DIR" "${TIDY_SOURCES[@]}"
else
   NLOHMANN_INCLUDE="$BUILD_DIR/_deps/nlohmann_json-src/include"
   RunClangTidy "${TIDY_SOURCES[@]}" -- -std=c++23 -Wno-everything -Iinclude -Iexamples/TodoList/core/include \
      -Iexamples/TodoList/methods/include -isystem "$NLOHMANN_INCLUDE"
fi
