#!/usr/bin/env bash
# ==============================================================================
# Filament Engine — Setup
#
# Downloads and builds all third-party dependencies, then configures CMake.
# Run this script once after cloning the repository.
#
# Dependencies:
#   - EnTT v3.16.0 (header-only ECS library)
#   - Google Filament (3D rendering engine, built from source)
#
# Usage:
#   ./scripts/setup.sh          # Full setup (download + build + configure)
#   ./scripts/setup.sh --skip-filament-build  # Skip Filament build (if already built)
# ==============================================================================
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
VENDOR_DIR="${PROJECT_ROOT}/vendor"
BUILD_DIR="${PROJECT_ROOT}/build"

# Versions
ENTT_VERSION="v3.16.0"
FILAMENT_TAG="v1.56.3"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

info()    { echo -e "${GREEN}[INFO]${NC} $1"; }
warn()    { echo -e "${YELLOW}[WARN]${NC} $1"; }
error()   { echo -e "${RED}[ERROR]${NC} $1"; }

SKIP_FILAMENT_BUILD=false
for arg in "$@"; do
    case "$arg" in
        --skip-filament-build) SKIP_FILAMENT_BUILD=true ;;
        *) error "Unknown argument: $arg"; exit 1 ;;
    esac
done

echo "==========================================="
echo " Filament Engine — Setup"
echo "==========================================="
echo ""
info "Project root: ${PROJECT_ROOT}"

# ==============================================================================
# 1. Create vendor directory
# ==============================================================================
mkdir -p "${VENDOR_DIR}"

# ==============================================================================
# 2. Download EnTT (header-only)
# ==============================================================================
ENTT_DIR="${VENDOR_DIR}/entt"

if [ -d "${ENTT_DIR}/single_include/entt" ]; then
    info "EnTT already present, skipping download."
else
    info "Downloading EnTT ${ENTT_VERSION}..."
    git clone --depth 1 --branch "${ENTT_VERSION}" \
        https://github.com/skypjack/entt.git "${ENTT_DIR}"
    info "EnTT ${ENTT_VERSION} downloaded successfully."
fi

# ==============================================================================
# 3. Download and build Google Filament
# ==============================================================================
FILAMENT_DIR="${VENDOR_DIR}/filament"
FILAMENT_DIST="${FILAMENT_DIR}/out/release/filament"

if [ -d "${FILAMENT_DIR}" ]; then
    info "Filament source already present."
else
    info "Cloning Google Filament ${FILAMENT_TAG}..."
    git clone --depth 1 --branch "${FILAMENT_TAG}" \
        https://github.com/google/filament.git "${FILAMENT_DIR}"
    info "Filament ${FILAMENT_TAG} cloned successfully."
fi

if [ "${SKIP_FILAMENT_BUILD}" = true ]; then
    warn "Skipping Filament build (--skip-filament-build)."
elif [ -d "${FILAMENT_DIST}" ]; then
    info "Filament distribution already built at: ${FILAMENT_DIST}"
    info "To rebuild, remove ${FILAMENT_DIST} and re-run setup."
else
    info "Building Filament (this may take a while)..."
    cd "${FILAMENT_DIR}"

    # Filament's build script handles platform detection
    ./build.sh -i release

    if [ -d "${FILAMENT_DIST}" ]; then
        info "Filament built successfully at: ${FILAMENT_DIST}"
    else
        error "Filament build failed — distribution not found at: ${FILAMENT_DIST}"
        exit 1
    fi
fi

# ==============================================================================
# 4. Configure CMake
# ==============================================================================
info "Configuring CMake..."
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"
cmake "${PROJECT_ROOT}" -G Ninja

echo ""
echo "==========================================="
info "Setup complete!"
echo ""
echo "  Dependencies:"
echo "    • EnTT ${ENTT_VERSION}  → ${ENTT_DIR}"
echo "    • Filament ${FILAMENT_TAG} → ${FILAMENT_DIR}"
echo ""
echo "  Next steps:"
echo "    ./scripts/build.sh      # Build the engine"
echo "    ./scripts/runtest.sh    # Run all tests"
echo "==========================================="
