#!/usr/bin/env bash
# Shared test helper used by job-specific unit-test scripts.
# See also: ci-build.sh, ci-lint.sh, and ci-all.sh.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

if [[ -z "${CMAKE_PRESET:-}" && -z "${BUILD_DIR:-}" ]]; then
   CMAKE_PRESET="$(DefaultCMakePreset)"
fi

BUILD_TYPE="${BUILD_TYPE:-Debug}"
BUILD_CONFIG="${BUILD_CONFIG:-$BUILD_TYPE}"

if [[ "${SKIP_BUILD:-0}" != "1" ]]; then
   export CMAKE_PRESET
   bash "$SCRIPT_DIR/ci-build.sh"
fi

if [[ -n "${CMAKE_PRESET:-}" ]]; then
   TEST_PRESET="${TEST_PRESET:-$CMAKE_PRESET}"

   echo "==> ctest ($TEST_PRESET)"
   ctest --preset "$TEST_PRESET"
else
   BUILD_DIR="${BUILD_DIR:-build}"

   echo "==> ctest"
   ctest --test-dir "$BUILD_DIR" -C "$BUILD_CONFIG" --output-on-failure
fi
