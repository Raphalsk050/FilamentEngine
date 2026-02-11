#!/usr/bin/env bash
# Setup: creates build directory and runs CMake configuration
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"

echo "=== Filament Engine — Setup ==="
echo "Project root: ${PROJECT_ROOT}"

# Create build directory
mkdir -p "${BUILD_DIR}"

# Run CMake
echo "Running CMake..."
cd "${BUILD_DIR}"
cmake "${PROJECT_ROOT}" -G Ninja

echo ""
echo "✅ Setup complete. Run ./scripts/build.sh to build."
