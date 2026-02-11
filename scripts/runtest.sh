#!/usr/bin/env bash
# Run Tests: builds and runs all CTest tests
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"

# Build first to ensure everything is up to date
"${PROJECT_ROOT}/scripts/build.sh"

echo ""
echo "=== Filament Engine — Tests ==="

cd "${BUILD_DIR}"
ctest --output-on-failure "$@"

echo ""
echo "✅ Tests complete."
