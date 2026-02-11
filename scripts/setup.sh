#!/usr/bin/env bash
# ==============================================================================
# Filament Engine — Setup (robust)
#
# - Downloads third-party dependencies (EnTT, Filament prebuilt)
# - Installs system prerequisites (cmake/ninja/git/curl, SDL2, etc.)
# - Fixes CRLF issues and patches build.sh CPU detection for Linux/WSL
# - Optionally installs LLVM/Clang (Ubuntu/WSL) and configures CMake toolchain
#
# Usage:
#   ./scripts/setup.sh
#   ./scripts/setup.sh --no-cmake
#   ./scripts/setup.sh --deps-only
#
# Optional compiler:
#   ./scripts/setup.sh --llvm 21 --use-clang          # install clang-21 (Ubuntu) + use it in CMake
#   ./scripts/setup.sh --use-clang                    # use clang if present (no install)
#   ./scripts/setup.sh --compiler gcc                 # force gcc/g++
#
# Notes:
# - LLVM installation uses apt.llvm.org (downloads llvm.sh script).
#   If you prefer, install clang from distro repos and use --use-clang.
# ==============================================================================
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
VENDOR_DIR="${PROJECT_ROOT}/vendor"
BUILD_DIR="${PROJECT_ROOT}/build"
SCRIPTS_DIR="${PROJECT_ROOT}/scripts"

# -------------------------
# Dependency versions
# -------------------------
ENTT_VERSION="v3.16.0"
FILAMENT_VERSION="v1.69.2"

# -------------------------
# Defaults / flags
# -------------------------
SKIP_CMAKE=false
DEPS_ONLY=false

WANT_USE_CLANG=true
FORCED_COMPILER="clang"     # "clang" or "gcc" or ""
LLVM_VERSION="21"        # e.g. "21"
LLVM_SYMLINK=true      # create /usr/local/bin/clang -> clang-XX

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

die() { error "$1"; exit 1; }

# -------------------------
# Temp dir (auto-cleanup)
# -------------------------
TMP_DIR="$(mktemp -d 2>/dev/null || mktemp -d -t filament_setup)"
cleanup() { rm -rf "${TMP_DIR}" 2>/dev/null || true; }
trap cleanup EXIT

# -------------------------
# Parse arguments
# -------------------------
while [[ $# -gt 0 ]]; do
  case "$1" in
    --no-cmake) SKIP_CMAKE=true; shift ;;
    --deps-only) SKIP_CMAKE=true; DEPS_ONLY=true; shift ;;
    --use-clang) WANT_USE_CLANG=true; shift ;;
    --compiler)
      [[ $# -lt 2 ]] && die "Missing value for --compiler (use 'clang' or 'gcc')"
      FORCED_COMPILER="$2"
      shift 2
      ;;
    --llvm)
      [[ $# -lt 2 ]] && die "Missing value for --llvm (e.g. 21)"
      LLVM_VERSION="$2"
      shift 2
      ;;
    --no-llvm-symlink) LLVM_SYMLINK=false; shift ;;
    -h|--help)
      cat <<EOF
Usage: $0 [OPTIONS]

Options:
  --no-cmake               Download/build dependencies only (skip CMake configure)
  --deps-only              Same as --no-cmake
  --use-clang              Use clang/clang++ for CMake if available
  --compiler clang|gcc     Force compiler choice for CMake
  --llvm <ver>             Install LLVM/Clang version (Ubuntu/WSL only), e.g. 21
  --no-llvm-symlink        Don't create /usr/local/bin/clang symlinks when using --llvm
  -h, --help               Show this help
EOF
      exit 0
      ;;
    *)
      die "Unknown argument: $1"
      ;;
  esac
done

# -------------------------
# Detect platform
# -------------------------
OS="$(uname -s)"
ARCH="$(uname -m)"

is_wsl() {
  [[ -n "${WSL_INTEROP:-}" ]] && return 0
  [[ -n "${WSL_DISTRO_NAME:-}" ]] && return 0
  grep -qiE "(microsoft|wsl)" /proc/version 2>/dev/null && return 0
  return 1
}

PLATFORM="unknown"
case "${OS}" in
  Darwin) PLATFORM="macos" ;;
  Linux)  PLATFORM="linux" ;;
  *)      PLATFORM="unknown" ;;
esac

echo "==========================================="
echo " Filament Engine — Setup"
echo "==========================================="
info "Project root: ${PROJECT_ROOT}"
info "Platform: ${OS} (${ARCH})"
if [[ "${PLATFORM}" == "linux" ]] && is_wsl; then
  info "Environment: WSL detected"
fi

# -------------------------
# Utilities
# -------------------------
need_cmd() {
  command -v "$1" >/dev/null 2>&1 || return 1
}

require_cmd() {
  need_cmd "$1" || die "Missing required tool: $1"
}

apt_install() {
  local pkgs=("$@")
  sudo apt-get update -y
  sudo apt-get install -y "${pkgs[@]}"
}

brew_install() {
  local pkgs=("$@")
  brew update
  brew install "${pkgs[@]}"
}

ensure_prereqs() {
  section "System prerequisites"

  if [[ "${PLATFORM}" == "macos" ]]; then
    if ! need_cmd brew; then
      warn "Homebrew not found. Please install Homebrew first."
      warn "Then re-run setup. (Need: git, cmake, ninja, curl, sdl2)"
      return 0
    fi

    local want=()
    need_cmd git   || want+=("git")
    need_cmd cmake || want+=("cmake")
    need_cmd ninja || want+=("ninja")
    need_cmd curl  || want+=("curl")
    # SDL2 handled later (but can pre-install)
    if [[ ${#want[@]} -gt 0 ]]; then
      info "Installing via Homebrew: ${want[*]}"
      brew_install "${want[@]}"
    else
      info "Core tools already installed. ✓"
    fi

  elif [[ "${PLATFORM}" == "linux" ]]; then
    if ! need_cmd apt-get; then
      warn "apt-get not found. Please install prerequisites manually:"
      warn "git curl ca-certificates tar cmake ninja-build pkg-config"
      return 0
    fi

    local want=()
    need_cmd git     || want+=("git")
    need_cmd curl    || want+=("curl")
    need_cmd tar     || want+=("tar")
    need_cmd cmake   || want+=("cmake")
    need_cmd ninja   || want+=("ninja-build")
    need_cmd pkg-config || want+=("pkg-config")
    # helpful base toolchain (optional, but often needed)
    # (we install build-essential if missing gcc/g++)
    if ! need_cmd gcc || ! need_cmd g++; then
      want+=("build-essential")
    fi
    # certificates for https
    want+=("ca-certificates")

    if [[ ${#want[@]} -gt 0 ]]; then
      info "Installing via apt: ${want[*]}"
      apt_install "${want[@]}"
    else
      info "Core tools already installed. ✓"
    fi
  else
    warn "Unsupported platform: ${OS}. Continuing without auto-install."
  fi
}

normalize_line_endings() {
  section "Normalize line endings (CRLF -> LF)"

  local changed=false
  shopt -s nullglob
  for f in "${SCRIPTS_DIR}"/*.sh; do
    if grep -q $'\r' "$f" 2>/dev/null; then
      warn "CRLF detected in: ${f##*/} — converting to LF"
      # Safe in-place CRLF removal
      tr -d '\r' < "$f" > "${TMP_DIR}/_tmpfile"
      cat "${TMP_DIR}/_tmpfile" > "$f"
      chmod +x "$f" || true
      changed=true
    fi
  done
  shopt -u nullglob

  if [[ "${changed}" == false ]]; then
    info "All scripts already use LF. ✓"
  fi
}

patch_build_script() {
  section "Patch build.sh (CPU detection portability)"

  local f="${SCRIPTS_DIR}/build.sh"
  if [[ ! -f "${f}" ]]; then
    warn "build.sh not found, skipping patch."
    return 0
  fi

  # Fix shebang if someone accidentally wrote "# !/usr/bin/env bash"
  if head -n 1 "${f}" | grep -qE '^#\s+!/usr/bin/env bash'; then
    warn "Fixing build.sh shebang (removing space)"
    perl -pi -e 's/^#\s+(!\/usr\/bin\/env bash)/#\1/' "${f}"
  fi

  # Patch the known hw.ncpu logic that breaks on Linux/WSL
  if grep -q "hw\.ncpu" "${f}"; then
    warn "Detected sysctl hw.ncpu logic in build.sh — patching to uname-based detection"

    perl -0777 -pi -e '
      s/# Detect CPU count\s*if command -v sysctl &>\/dev\/null; then\s*JOBS=\$\(sysctl -n hw\.ncpu\)\s*else\s*JOBS=\$\(nproc 2>\/dev\/null \|\| echo 4\)\s*fi/
# Detect CPU count (portable)
if [[ "$(uname -s)" == "Darwin" ]]; then
    JOBS="$(sysctl -n hw.ncpu)"
else
    JOBS="$(nproc 2>\/dev\/null || getconf _NPROCESSORS_ONLN 2>\/dev\/null || echo 4)"
fi/sm
    ' "${f}" || true
  else
    info "build.sh CPU detection looks OK. ✓"
  fi
}

install_llvm_ubuntu() {
  local ver="$1"

  if [[ "${PLATFORM}" != "linux" ]]; then
    warn "LLVM install requested, but platform is not Linux. Skipping."
    return 0
  fi

  if ! need_cmd apt-get; then
    warn "LLVM install requested, but apt-get not found. Skipping."
    return 0
  fi

  section "Install LLVM/Clang ${ver} (apt.llvm.org)"

  # If already installed, skip
  if need_cmd "clang-${ver}" && need_cmd "clang++-${ver}"; then
    info "clang-${ver} already installed. ✓"
  else
    info "Downloading llvm.sh..."
    curl -fsSL -o "${TMP_DIR}/llvm.sh" "https://apt.llvm.org/llvm.sh"
    chmod +x "${TMP_DIR}/llvm.sh"

    info "Installing LLVM/Clang ${ver}..."
    # llvm.sh handles repo + key + packages
    sudo "${TMP_DIR}/llvm.sh" "${ver}"
  fi

  # Explicitly install libc++ and libc++abi if using Clang
  # This is needed even if Clang was already installed, because often dev headers are missing
  if [[ "${PLATFORM}" == "linux" ]]; then
     info "Ensuring libc++ dependencies are installed for Clang ${ver}..."
     # We use DEBIAN_FRONTEND=noninteractive to avoid potential prompts
     sudo DEBIAN_FRONTEND=noninteractive apt-get install -y "libc++-${ver}-dev" "libc++abi-${ver}-dev" || warn "Failed to install libc++ dev packages. You may need to install them manually."
  fi

  if [[ "${LLVM_SYMLINK}" == true ]]; then
    section "Symlink clang -> clang-${ver}"

    local cbin
    cbin="$(command -v "clang-${ver}" || true)"
    local cxbin
    cxbin="$(command -v "clang++-${ver}" || true)"

    [[ -z "${cbin}" || -z "${cxbin}" ]] && die "clang-${ver} not found after install."

    sudo mkdir -p /usr/local/bin
    sudo ln -sf "${cbin}" /usr/local/bin/clang
    sudo ln -sf "${cxbin}" /usr/local/bin/clang++

    info "Symlinks created:"
    info "  /usr/local/bin/clang  -> ${cbin}"
    info "  /usr/local/bin/clang++ -> ${cxbin}"
  else
    warn "Skipping symlink creation (--no-llvm-symlink)."
  fi
}

choose_compiler_for_cmake() {
  # Priority:
  # 1) --compiler gcc|clang
  # 2) --use-clang
  # 3) default: whatever cmake finds
  local cc="" cxx=""

  if [[ -n "${FORCED_COMPILER}" ]]; then
    if [[ "${FORCED_COMPILER}" == "clang" ]]; then
      cc="$(command -v clang || true)"
      cxx="$(command -v clang++ || true)"
    elif [[ "${FORCED_COMPILER}" == "gcc" ]]; then
      cc="$(command -v gcc || true)"
      cxx="$(command -v g++ || true)"
    else
      die "Invalid --compiler value: ${FORCED_COMPILER} (use 'clang' or 'gcc')"
    fi
  elif [[ "${WANT_USE_CLANG}" == true ]]; then
    cc="$(command -v clang || true)"
    cxx="$(command -v clang++ || true)"
  fi

  if [[ -n "${cc}" && -n "${cxx}" ]]; then
    echo "${cc};${cxx}"
  else
    echo ";"
  fi
}

# ==============================================================================
# 0. Pre-flight: normalize scripts, patch build.sh, install prereqs, install LLVM
# ==============================================================================
normalize_line_endings
patch_build_script
ensure_prereqs

# If requested, install LLVM/Clang (Ubuntu/WSL)
if [[ -n "${LLVM_VERSION}" ]]; then
  install_llvm_ubuntu "${LLVM_VERSION}"
fi

# ==============================================================================
# 1. Create vendor directory
# ==============================================================================
mkdir -p "${VENDOR_DIR}"

# ==============================================================================
# 2. EnTT (header-only, cloned from GitHub)
# ==============================================================================
section "EnTT ${ENTT_VERSION}"
ENTT_DIR="${VENDOR_DIR}/entt"

if [[ -d "${ENTT_DIR}/single_include/entt" ]]; then
  info "EnTT already present, skipping."
else
  require_cmd git
  info "Cloning EnTT ${ENTT_VERSION}..."
  git clone --depth 1 --branch "${ENTT_VERSION}" \
    https://github.com/skypjack/entt.git "${ENTT_DIR}"
  info "EnTT ${ENTT_VERSION} ✓"
fi

# ==============================================================================
# 3. Google Filament (pre-built)
# ==============================================================================
section "Filament ${FILAMENT_VERSION}"
FILAMENT_DIR="${VENDOR_DIR}/filament"
FILAMENT_OUT="${FILAMENT_DIR}/out/release"
FILAMENT_DIST="${FILAMENT_OUT}/filament"

if [[ -d "${FILAMENT_DIST}/include" && -d "${FILAMENT_DIST}/lib" ]]; then
  info "Filament distribution already present at: ${FILAMENT_DIST}"
  info "Skipping."
else
  require_cmd curl
  require_cmd tar

  case "${OS}" in
    Darwin) PLATFORM_SUFFIX="mac" ;;
    Linux)  PLATFORM_SUFFIX="linux" ;;
    *)      die "Unsupported OS: ${OS}" ;;
  esac

  TARBALL_NAME="filament-${FILAMENT_VERSION}-${PLATFORM_SUFFIX}.tgz"
  DOWNLOAD_URL="https://github.com/google/filament/releases/download/${FILAMENT_VERSION}/${TARBALL_NAME}"
  TARBALL_PATH="${TMP_DIR}/${TARBALL_NAME}"

  info "Downloading pre-built Filament ${FILAMENT_VERSION} for ${OS}..."
  info "URL: ${DOWNLOAD_URL}"

  # More robust curl (retries)
  if curl -fSL --retry 3 --retry-delay 2 --progress-bar -o "${TARBALL_PATH}" "${DOWNLOAD_URL}"; then
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

  mkdir -p "${FILAMENT_OUT}"
  info "Extracting Filament..."
  tar -xzf "${TARBALL_PATH}" -C "${FILAMENT_OUT}"

  # Find extracted dist that contains include/ and lib/
  if [[ -d "${FILAMENT_DIST}/include" && -d "${FILAMENT_DIST}/lib" ]]; then
    info "Filament extracted to expected path. ✓"
  else
    # Try to locate a directory that looks like filament dist
    CANDIDATE="$(find "${FILAMENT_OUT}" -maxdepth 2 -type d -name filament 2>/dev/null | head -n 1 || true)"
    if [[ -n "${CANDIDATE}" && -d "${CANDIDATE}/include" && -d "${CANDIDATE}/lib" ]]; then
      mkdir -p "${FILAMENT_OUT}"
      if [[ "${CANDIDATE}" != "${FILAMENT_DIST}" ]]; then
        rm -rf "${FILAMENT_DIST}" 2>/dev/null || true
        mv "${CANDIDATE}" "${FILAMENT_DIST}"
      fi
    else
      # Fallback: find any dir containing include & lib
      CANDIDATE="$(find "${FILAMENT_OUT}" -maxdepth 2 -type d 2>/dev/null | while read -r d; do
        if [[ -d "${d}/include" && -d "${d}/lib" ]]; then echo "${d}"; break; fi
      done)"
      if [[ -n "${CANDIDATE}" ]]; then
        rm -rf "${FILAMENT_DIST}" 2>/dev/null || true
        mv "${CANDIDATE}" "${FILAMENT_DIST}"
      else
        error "Extraction failed: Filament dist not found."
        error "Contents of ${FILAMENT_OUT}:"
        ls -la "${FILAMENT_OUT}" 2>/dev/null || true
        exit 1
      fi
    fi
  fi

  if [[ -d "${FILAMENT_DIST}/include" && -d "${FILAMENT_DIST}/lib" ]]; then
    info "Filament ${FILAMENT_VERSION} ✓"
  else
    die "Filament dist invalid after extraction."
  fi
fi

# ==============================================================================
# 4. SDL2 (required by pre-built Filament)
# ==============================================================================
section "SDL2"

if [[ "${PLATFORM}" == "macos" ]]; then
  if need_cmd brew; then
    if brew list sdl2 &>/dev/null; then
      info "SDL2 installed via Homebrew. ✓"
    else
      info "Installing SDL2 via Homebrew..."
      brew_install sdl2
      info "SDL2 installed. ✓"
    fi
  else
    warn "Homebrew not found. Please install SDL2 manually."
  fi
elif [[ "${PLATFORM}" == "linux" ]]; then
  if need_cmd apt-get; then
    if dpkg -s libsdl2-dev &>/dev/null 2>&1; then
      info "SDL2 installed via apt. ✓"
    else
      info "Installing SDL2 via apt..."
      apt_install libsdl2-dev
      info "SDL2 installed. ✓"
    fi
  else
    warn "apt-get not found. Please install SDL2 manually."
  fi
else
  warn "Unknown platform: cannot install SDL2 automatically."
fi

# ==============================================================================
# 5. Configure CMake
# ==============================================================================
if [[ "${SKIP_CMAKE}" == true ]]; then
  warn "Skipping CMake configuration (--no-cmake)."
else
  section "CMake Configuration"

  require_cmd cmake
  require_cmd ninja

  mkdir -p "${BUILD_DIR}"

  IFS=';' read -r CC_PATH CXX_PATH <<< "$(choose_compiler_for_cmake)"

  CMAKE_ARGS=(
    "-S" "${PROJECT_ROOT}"
    "-B" "${BUILD_DIR}"
    "-G" "Ninja"
    "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
  )

  if [[ -n "${CC_PATH}" && -n "${CXX_PATH}" ]]; then
    info "Using compiler:"
    info "  CC:  ${CC_PATH}"
    info "  CXX: ${CXX_PATH}"
    CMAKE_ARGS+=("-DCMAKE_C_COMPILER=${CC_PATH}")
    CMAKE_ARGS+=("-DCMAKE_CXX_COMPILER=${CXX_PATH}")
  else
    info "Using default compiler detected by CMake."
  fi

  cmake "${CMAKE_ARGS[@]}"
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
