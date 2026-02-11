#!/usr/bin/env bash
# Build: compiles the project using Ninja
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"

# Check if build was configured
if [ ! -f "${BUILD_DIR}/build.ninja" ]; then
    echo "Build not configured. Running setup first..."
    "${PROJECT_ROOT}/scripts/setup.sh"
fi

# Detect CPU count
if command -v sysctl &>/dev/null; then
    JOBS=$(sysctl -n hw.ncpu)
else
    JOBS=$(nproc 2>/dev/null || echo 4)
fi

echo "=== Filament Engine — Build (${JOBS} jobs) ==="

ninja -C "${BUILD_DIR}" -j"${JOBS}"

echo ""
echo "✅ Build complete. Binary at: ${BUILD_DIR}/sandbox/sandbox"
