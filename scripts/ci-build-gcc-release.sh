#!/usr/bin/env bash
# Mirrors GitHub Actions job: build (gcc release).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

CMAKE_PRESET=gcc-release bash "$SCRIPT_DIR/ci-build.sh"
