#!/usr/bin/env bash
# Mirrors GitHub Actions job: unit-tests (clang).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

CMAKE_PRESET=clang-debug bash "$SCRIPT_DIR/ci-unit-tests.sh"
