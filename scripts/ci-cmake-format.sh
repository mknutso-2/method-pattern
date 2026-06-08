#!/usr/bin/env bash
# Mirrors GitHub Actions job: cmake-format.

set -euo pipefail

RunCMakeFormat() {
   if [[ -n "${CMAKE_FORMAT:-}" ]]; then
      "$CMAKE_FORMAT" "$@"
      return
   fi

   if command -v cmake-format >/dev/null 2>&1; then
      cmake-format "$@"
      return
   fi

   if command -v python3 >/dev/null 2>&1 && python3 -m cmakelang.format --version >/dev/null 2>&1; then
      python3 -m cmakelang.format "$@"
      return
   fi

   if command -v python >/dev/null 2>&1 && python -m cmakelang.format --version >/dev/null 2>&1; then
      python -m cmakelang.format "$@"
      return
   fi

   if command -v py >/dev/null 2>&1 && py -m cmakelang.format --version >/dev/null 2>&1; then
      py -m cmakelang.format "$@"
      return
   fi

   printf "ERROR: cmake-format is required. Install cmakelang or set CMAKE_FORMAT.\n" >&2
   exit 1
}

mapfile -t CMAKE_FILES < <(
   find . \
      \( -path "./.git" -o -path "./build" -o -path "./build-*" -o -path "./.cache" -o -path "./.ci-tools" -o -path "./out" \) -prune \
      -o -type f \( -name "CMakeLists.txt" -o -name "*.cmake" \) -print | sort
)

if [[ "${#CMAKE_FILES[@]}" -eq 0 ]]; then
   exit 0
fi

echo "==> cmake-format"
RunCMakeFormat --check "${CMAKE_FILES[@]}"
