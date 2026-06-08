#!/usr/bin/env bash
# Run CI-like local checks for the current platform.

set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

status=0

RunRequired() {
   local label="$1"
   local script_name="$2"

   printf "\n==> required: %s\n" "$label"
   if ! bash "$SCRIPT_DIR/$script_name"; then
      printf "ERROR: required check failed: %s\n" "$label" >&2
      status=1
   fi
}

RunOptional() {
   local label="$1"
   local script_name="$2"
   shift 2

   local missing_commands=()
   local command_name
   for command_name in "$@"; do
      if ! CommandExists "$command_name"; then
         missing_commands+=("$command_name")
      fi
   done

   if [[ "${#missing_commands[@]}" -gt 0 ]]; then
      printf "\nWARNING: skipping optional check '%s'; missing command(s): %s\n" \
         "$label" "${missing_commands[*]}" >&2
      return 0
   fi

   printf "\n==> optional: %s\n" "$label"
   if ! bash "$SCRIPT_DIR/$script_name"; then
      printf "WARNING: optional check failed: %s\n" "$label" >&2
   fi
}

RunRequired "cmake-format" "ci-cmake-format.sh"
RunRequired "lint" "ci-lint.sh"
RunRequired "public-headers" "ci-public-headers.sh"

if IsWindows; then
   RunRequired "build (msvc)" "ci-build-msvc.sh"
   RunRequired "build (msvc release)" "ci-build-msvc-release.sh"
   RunRequired "unit-tests (msvc)" "ci-unit-tests-msvc.sh"
   RunRequired "unit-tests (msvc release)" "ci-unit-tests-msvc-release.sh"

   RunOptional "build (gcc)" "ci-build-gcc.sh" gcc g++ ninja
   RunOptional "build (gcc release)" "ci-build-gcc-release.sh" gcc g++ ninja
   RunOptional "unit-tests (gcc)" "ci-unit-tests-gcc.sh" gcc g++ ninja
   RunOptional "unit-tests (gcc release)" "ci-unit-tests-gcc-release.sh" gcc g++ ninja
   RunOptional "build (clang)" "ci-build-clang.sh" clang clang++ ninja
   RunOptional "build (clang release)" "ci-build-clang-release.sh" clang clang++ ninja
   RunOptional "unit-tests (clang)" "ci-unit-tests-clang.sh" clang clang++ ninja
   RunOptional "unit-tests (clang release)" "ci-unit-tests-clang-release.sh" clang clang++ ninja
   RunOptional "sanitizers (clang)" "ci-sanitizers-clang.sh" clang clang++ ninja
elif IsLinux; then
   RunRequired "build (gcc)" "ci-build-gcc.sh"
   RunRequired "build (gcc release)" "ci-build-gcc-release.sh"
   RunRequired "unit-tests (gcc)" "ci-unit-tests-gcc.sh"
   RunRequired "unit-tests (gcc release)" "ci-unit-tests-gcc-release.sh"
   RunRequired "build (clang)" "ci-build-clang.sh"
   RunRequired "build (clang release)" "ci-build-clang-release.sh"
   RunRequired "unit-tests (clang)" "ci-unit-tests-clang.sh"
   RunRequired "unit-tests (clang release)" "ci-unit-tests-clang-release.sh"
   RunRequired "sanitizers (clang)" "ci-sanitizers-clang.sh"
else
   RunOptional "build (gcc)" "ci-build-gcc.sh" gcc g++ ninja
   RunOptional "build (gcc release)" "ci-build-gcc-release.sh" gcc g++ ninja
   RunOptional "unit-tests (gcc)" "ci-unit-tests-gcc.sh" gcc g++ ninja
   RunOptional "unit-tests (gcc release)" "ci-unit-tests-gcc-release.sh" gcc g++ ninja
   RunOptional "build (clang)" "ci-build-clang.sh" clang clang++ ninja
   RunOptional "build (clang release)" "ci-build-clang-release.sh" clang clang++ ninja
   RunOptional "unit-tests (clang)" "ci-unit-tests-clang.sh" clang clang++ ninja
   RunOptional "unit-tests (clang release)" "ci-unit-tests-clang-release.sh" clang clang++ ninja
   RunOptional "sanitizers (clang)" "ci-sanitizers-clang.sh" clang clang++ ninja
fi

exit "$status"
