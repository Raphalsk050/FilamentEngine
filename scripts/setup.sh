#!/usr/bin/env bash
# ==============================================================================
# Filament Engine — Setup
#
# Downloads all third-party dependencies and configures CMake.
# Run this script once after cloning the repository.
#
# Dependencies:
#   - EnTT v3.16.0 (header-only ECS library, cloned from GitHub)
#   - Google Filament v1.69.2 (rendering engine, pre-built from GitHub Releases)
#   - SDL2 (installed via Homebrew/apt as fallback when using pre-built Filament)
#
# Usage:
#   ./scripts/setup.sh              # Full setup (download + build + configure)
#   ./scripts/setup.sh --no-cmake   # Download/build dependencies only
#   ./scripts/setup.sh --deps-only  # Same as --no-cmake
# ==============================================================================
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
VENDOR_DIR="${PROJECT_ROOT}/vendor"
BUILD_DIR="${PROJECT_ROOT}/build"

# -------------------------
# Dependency versions
# -------------------------
ENTT_VERSION="v3.16.0"
FILAMENT_VERSION="v1.69.2"

# -------------------------
# Colors
# -------------------------
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

info()    { echo -e "${GREEN}[INFO]${NC} $1"; }
warn()    { echo -e "${YELLOW}[WARN]${NC} $1"; }
error()   { echo -e "${RED}[ERROR]${NC} $1"; }
section() { echo -e "\n${CYAN}=== $1 ===${NC}"; }

# -------------------------
# Parse arguments
# -------------------------
SKIP_CMAKE=false
for arg in "$@"; do
    case "$arg" in
        --no-cmake|--deps-only) SKIP_CMAKE=true ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --no-cmake, --deps-only   Download/build dependencies only"
            echo "  -h, --help                Show this help"
            exit 0
            ;;
        *) error "Unknown argument: $arg"; exit 1 ;;
    esac
done

# -------------------------
# Detect platform
# -------------------------
OS="$(uname -s)"
ARCH="$(uname -m)"

echo "==========================================="
echo " Filament Engine — Setup"
echo "==========================================="
info "Project root: ${PROJECT_ROOT}"
info "Platform: ${OS} (${ARCH})"

# ==============================================================================
# 1. Create vendor directory
# ==============================================================================
mkdir -p "${VENDOR_DIR}"

# ==============================================================================
# 2. EnTT (header-only, cloned from GitHub)
# ==============================================================================
section "EnTT ${ENTT_VERSION}"
ENTT_DIR="${VENDOR_DIR}/entt"

if [ -d "${ENTT_DIR}/single_include/entt" ]; then
    info "EnTT already present, skipping."
else
    info "Cloning EnTT ${ENTT_VERSION}..."
    git clone --depth 1 --branch "${ENTT_VERSION}" \
        https://github.com/skypjack/entt.git "${ENTT_DIR}"
    info "EnTT ${ENTT_VERSION} ✓"
fi

# ==============================================================================
# 3. Google Filament
# ==============================================================================
section "Filament ${FILAMENT_VERSION}"

FILAMENT_DIR="${VENDOR_DIR}/filament"
FILAMENT_DIST="${FILAMENT_DIR}/out/release/filament"

if [ -d "${FILAMENT_DIST}/include" ] && [ -d "${FILAMENT_DIST}/lib" ]; then
    info "Filament distribution already present at: ${FILAMENT_DIST}"
    info "Skipping."
else
    # Determine platform-specific tarball name
    case "${OS}" in
        Darwin) PLATFORM_SUFFIX="mac" ;;
        Linux)  PLATFORM_SUFFIX="linux" ;;
        *)      error "Unsupported OS: ${OS}"; exit 1 ;;
    esac

    TARBALL_NAME="filament-${FILAMENT_VERSION}-${PLATFORM_SUFFIX}.tgz"
    DOWNLOAD_URL="https://github.com/google/filament/releases/download/${FILAMENT_VERSION}/${TARBALL_NAME}"
    TARBALL_PATH="${VENDOR_DIR}/${TARBALL_NAME}"

    info "Downloading pre-built Filament ${FILAMENT_VERSION} for ${OS}..."
    info "URL: ${DOWNLOAD_URL}"

    if curl -fSL --progress-bar -o "${TARBALL_PATH}" "${DOWNLOAD_URL}"; then
        info "Download complete."
    else
        error "Failed to download Filament from:"
        error "  ${DOWNLOAD_URL}"
        echo ""
        error "Alternative: build Filament from source:"
        error "  git clone --depth 1 --branch ${FILAMENT_VERSION} https://github.com/google/filament.git ${FILAMENT_DIR}"
        error "  cd ${FILAMENT_DIR} && ./build.sh -i release"
        exit 1
    fi

    # Extract tarball
    mkdir -p "${FILAMENT_DIR}/out/release"
    info "Extracting Filament..."
    tar -xzf "${TARBALL_PATH}" -C "${FILAMENT_DIR}/out/release"
    rm -f "${TARBALL_PATH}"

    # Handle if tarball extracts to a named subdirectory
    if [ ! -d "${FILAMENT_DIST}" ]; then
        EXTRACTED=$(find "${FILAMENT_DIR}/out/release" -maxdepth 1 -type d ! -name release | head -1)
        if [ -n "${EXTRACTED}" ]; then
            mv "${EXTRACTED}" "${FILAMENT_DIST}"
        else
            error "Extraction failed: ${FILAMENT_DIST} not found."
            error "Contents of out/release:"
            ls -la "${FILAMENT_DIR}/out/release/" 2>/dev/null || true
            exit 1
        fi
    fi
    info "Filament ${FILAMENT_VERSION} ✓"
fi

# ==============================================================================
# 4. SDL2 (fallback for pre-built Filament releases)
# ==============================================================================
section "SDL2"

# Pre-built Filament releases do not include SDL2 — install from system
{
    info "Checking for system SDL2..."

    if command -v brew &>/dev/null; then
        if brew list sdl2 &>/dev/null; then
            info "SDL2 installed via Homebrew. ✓"
        else
            info "Installing SDL2 via Homebrew..."
            brew install sdl2
            info "SDL2 installed. ✓"
        fi
    elif command -v apt-get &>/dev/null; then
        if dpkg -s libsdl2-dev &>/dev/null 2>&1; then
            info "SDL2 installed via apt. ✓"
        else
            info "Installing SDL2 via apt..."
            sudo apt-get update && sudo apt-get install -y libsdl2-dev
            info "SDL2 installed. ✓"
        fi
    else
        warn "Could not detect package manager."
        warn "Please install SDL2 manually before building."
    fi
}

# ==============================================================================
# 5. Configure CMake
# ==============================================================================
if [ "${SKIP_CMAKE}" = true ]; then
    warn "Skipping CMake configuration (--no-cmake)."
else
    section "CMake Configuration"
    mkdir -p "${BUILD_DIR}"
    cd "${BUILD_DIR}"
    cmake "${PROJECT_ROOT}" -G Ninja
    info "CMake configured. ✓"
fi

echo ""
echo "==========================================="
info "Setup complete!"
echo ""
echo "  Dependencies:"
echo "    • EnTT ${ENTT_VERSION}"
echo "    • Filament ${FILAMENT_VERSION} (pre-built)"
echo "    • SDL2"
echo ""
echo "  Next steps:"
echo "    ./scripts/build.sh      # Build the engine"
echo "    ./scripts/runtest.sh    # Run all tests"
echo "==========================================="
