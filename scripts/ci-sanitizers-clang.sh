#!/usr/bin/env bash
# Mirrors GitHub Actions job: sanitizers (clang).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

CMAKE_PRESET=clang-sanitizer-debug bash "$SCRIPT_DIR/ci-unit-tests.sh"
