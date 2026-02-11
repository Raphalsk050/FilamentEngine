#!/usr/bin/env bash
# ==============================================================================
# Filament Engine — Setup
#
# Downloads all third-party dependencies and configures CMake.
# Run this script once after cloning the repository.
#
# Dependencies downloaded:
#   - EnTT v3.16.0 (header-only ECS library, cloned from GitHub)
#   - Google Filament (pre-built release from GitHub Releases)
#   - SDL2 (installed via Homebrew as fallback for windowing)
#
# Usage:
#   ./scripts/setup.sh              # Full setup
#   ./scripts/setup.sh --no-cmake   # Download dependencies only (skip CMake)
# ==============================================================================
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
VENDOR_DIR="${PROJECT_ROOT}/vendor"
BUILD_DIR="${PROJECT_ROOT}/build"

# -------------------------
# Dependency versions
# -------------------------
ENTT_VERSION="v3.16.0"
FILAMENT_VERSION="v1.56.3"

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
        --no-cmake) SKIP_CMAKE=true ;;
        -h|--help)
            echo "Usage: $0 [--no-cmake]"
            echo "  --no-cmake   Download dependencies only, skip CMake configuration"
            exit 0
            ;;
        *) error "Unknown argument: $arg"; exit 1 ;;
    esac
done

# -------------------------
# Detect platform
# -------------------------
detect_platform() {
    local os arch
    os="$(uname -s)"
    arch="$(uname -m)"

    case "${os}" in
        Darwin)
            FILAMENT_PLATFORM="darwin"
            ;;
        Linux)
            FILAMENT_PLATFORM="linux"
            ;;
        *)
            error "Unsupported OS: ${os}"
            exit 1
            ;;
    esac

    info "Detected platform: ${os} (${arch})"
}

echo "==========================================="
echo " Filament Engine — Setup"
echo "==========================================="
info "Project root: ${PROJECT_ROOT}"
detect_platform

# ==============================================================================
# 1. Create vendor directory
# ==============================================================================
mkdir -p "${VENDOR_DIR}"

# ==============================================================================
# 2. Download EnTT (header-only, cloned from GitHub)
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
# 3. Download pre-built Filament from GitHub Releases
# ==============================================================================
section "Filament ${FILAMENT_VERSION}"

FILAMENT_DIR="${VENDOR_DIR}/filament"
FILAMENT_DIST="${FILAMENT_DIR}/out/release/filament"
FILAMENT_TARBALL_NAME="filament-${FILAMENT_VERSION}-${FILAMENT_PLATFORM}.tgz"
FILAMENT_URL="https://github.com/nickalcala/nickalcala-filament/releases/download/${FILAMENT_VERSION}/${FILAMENT_TARBALL_NAME}"
# Fallback to official Filament releases
FILAMENT_URL_OFFICIAL="https://github.com/nickalcala/nickalcala-filament/releases/download/${FILAMENT_VERSION}/${FILAMENT_TARBALL_NAME}"

if [ -d "${FILAMENT_DIST}" ]; then
    info "Filament distribution already present at: ${FILAMENT_DIST}"
    info "Skipping download."
else
    info "Downloading pre-built Filament ${FILAMENT_VERSION} for ${FILAMENT_PLATFORM}..."

    # Create target directory structure
    mkdir -p "${FILAMENT_DIR}/out/release"

    # Download tarball
    TARBALL_PATH="${VENDOR_DIR}/${FILAMENT_TARBALL_NAME}"

    # Try GitHub releases download
    DOWNLOAD_URL="https://github.com/google/filament/releases/download/${FILAMENT_VERSION}/${FILAMENT_TARBALL_NAME}"
    info "URL: ${DOWNLOAD_URL}"

    if curl -fSL --progress-bar -o "${TARBALL_PATH}" "${DOWNLOAD_URL}" 2>/dev/null; then
        info "Download complete."
    else
        error "Failed to download Filament from:"
        error "  ${DOWNLOAD_URL}"
        echo ""
        error "The pre-built release may not be available for your platform."
        error "Alternative: build Filament from source:"
        error "  git clone --depth 1 --branch ${FILAMENT_VERSION} https://github.com/google/filament.git ${FILAMENT_DIR}"
        error "  cd ${FILAMENT_DIR} && ./build.sh -i release"
        exit 1
    fi

    # Extract tarball
    info "Extracting Filament..."
    tar -xzf "${TARBALL_PATH}" -C "${FILAMENT_DIR}/out/release"

    # Clean up tarball
    rm -f "${TARBALL_PATH}"

    # Verify extraction
    if [ -d "${FILAMENT_DIST}" ]; then
        info "Filament ${FILAMENT_VERSION} extracted successfully."
    else
        # Some releases extract to a versioned directory; try to find it
        EXTRACTED=$(find "${FILAMENT_DIR}/out/release" -maxdepth 1 -type d ! -name release | head -1)
        if [ -n "${EXTRACTED}" ] && [ "${EXTRACTED}" != "${FILAMENT_DIST}" ]; then
            mv "${EXTRACTED}" "${FILAMENT_DIST}"
            info "Filament ${FILAMENT_VERSION} extracted successfully (renamed)."
        else
            error "Extraction failed — expected directory not found: ${FILAMENT_DIST}"
            error "Contents of out/release:"
            ls -la "${FILAMENT_DIR}/out/release/" 2>/dev/null || true
            exit 1
        fi
    fi

    info "Filament ${FILAMENT_VERSION} ✓"
fi

# ==============================================================================
# 4. SDL2 (fallback — needed for windowing/input when not using Filament's SDL2)
# ==============================================================================
section "SDL2"

# Check if Filament's built-in SDL2 exists (from a full source build)
SDL2_FILAMENT_LIB="${FILAMENT_DIR}/out/cmake-release/third_party/libsdl2/tnt/libsdl2.a"
if [ -f "${SDL2_FILAMENT_LIB}" ]; then
    info "SDL2 found in Filament build output. ✓"
else
    info "Filament's SDL2 not found (expected if using pre-built release)."
    info "Checking for system SDL2..."

    if command -v brew &>/dev/null; then
        if brew list sdl2 &>/dev/null; then
            info "SDL2 already installed via Homebrew. ✓"
        else
            info "Installing SDL2 via Homebrew..."
            brew install sdl2
            info "SDL2 installed. ✓"
        fi
    elif command -v apt-get &>/dev/null; then
        if dpkg -s libsdl2-dev &>/dev/null 2>&1; then
            info "SDL2 already installed via apt. ✓"
        else
            info "Installing SDL2 via apt..."
            sudo apt-get update && sudo apt-get install -y libsdl2-dev
            info "SDL2 installed. ✓"
        fi
    else
        warn "Could not detect package manager."
        warn "Please install SDL2 manually before building."
    fi
fi

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
