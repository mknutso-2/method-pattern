#!/usr/bin/env bash
# Shared build helper used by job-specific build scripts.
# See also: ci-lint.sh, ci-unit-tests.sh, and ci-all.sh.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

if [[ -z "${CMAKE_PRESET:-}" && -z "${BUILD_DIR:-}" ]]; then
   CMAKE_PRESET="$(DefaultCMakePreset)"
fi

BUILD_TYPE="${BUILD_TYPE:-Debug}"
BUILD_CONFIG="${BUILD_CONFIG:-$BUILD_TYPE}"

if [[ -n "${CMAKE_PRESET:-}" ]]; then
   BUILD_PRESET="${BUILD_PRESET:-$CMAKE_PRESET}"

   echo "==> cmake configure ($CMAKE_PRESET)"
   cmake --preset "$CMAKE_PRESET"

   echo "==> cmake build ($BUILD_PRESET)"
   cmake --build --preset "$BUILD_PRESET"
else
   BUILD_DIR="${BUILD_DIR:-build}"

   echo "==> cmake configure"
   cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DBUILD_TESTING=ON

   echo "==> cmake build"
   cmake --build "$BUILD_DIR" --config "$BUILD_CONFIG"
fi
