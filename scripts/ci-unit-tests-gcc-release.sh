#!/usr/bin/env bash
# Mirrors GitHub Actions job: unit-tests (gcc release).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

CMAKE_PRESET=gcc-release bash "$SCRIPT_DIR/ci-unit-tests.sh"
