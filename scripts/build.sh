#!/usr/bin/env bash
# Build: configures (if needed) and compiles the project using Ninja, forcing Clang.
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
SCRIPTS_DIR="${PROJECT_ROOT}/scripts"

# -------------------------
# Logging helpers
# -------------------------
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

info()  { echo -e "${GREEN}[INFO]${NC} $1"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $1"; }
error() { echo -e "${RED}[ERROR]${NC} $1"; }
die()   { error "$1"; exit 1; }

# -------------------------
# Args
# -------------------------
FORCE_RECONFIGURE=false
CLEAN=false
EXTRA_NINJA_ARGS=()

while [[ $# -gt 0 ]]; do
  case "$1" in
    --reconfigure) FORCE_RECONFIGURE=true; shift ;;
    --clean) CLEAN=true; shift ;;
    --) shift; EXTRA_NINJA_ARGS+=("$@"); break ;;
    -h|--help)
      cat <<EOF
Usage: $0 [OPTIONS] [-- <extra ninja args>]

Options:
  --clean         Remove build directory before configuring/building
  --reconfigure   Force CMake reconfigure with Clang even if already configured
  -h, --help      Show this help

Examples:
  ./scripts/build.sh
  ./scripts/build.sh --clean
  ./scripts/build.sh --reconfigure
  ./scripts/build.sh -- -k 0
EOF
      exit 0
      ;;
    *)
      die "Unknown argument: $1"
      ;;
  esac
done

# -------------------------
# CPU count (portable)
# -------------------------
get_cpu_count() {
  if [[ "$(uname -s)" == "Darwin" ]]; then
    sysctl -n hw.ncpu 2>/dev/null || echo 4
  else
    nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4
  fi
}

JOBS="$(get_cpu_count)"

# -------------------------
# Find best Clang
# Prefer clang-21 if available, otherwise clang.
# -------------------------
pick_clang() {
  local cc="" cxx=""

  if command -v clang-21 >/dev/null 2>&1 && command -v clang++-21 >/dev/null 2>&1; then
    cc="$(command -v clang-21)"
    cxx="$(command -v clang++-21)"
  elif command -v clang >/dev/null 2>&1 && command -v clang++ >/dev/null 2>&1; then
    cc="$(command -v clang)"
    cxx="$(command -v clang++)"
  else
    cc=""
    cxx=""
  fi

  echo "${cc};${cxx}"
}

IFS=';' read -r CLANG_C CLANG_CXX <<< "$(pick_clang)"

if [[ -z "${CLANG_C}" || -z "${CLANG_CXX}" ]]; then
  die "Clang not found. Install it (recommended): ./scripts/setup.sh --llvm 21 --use-clang"
fi

# -------------------------
# Basic tool checks
# -------------------------
command -v cmake >/dev/null 2>&1 || die "cmake not found."
command -v ninja >/dev/null 2>&1 || die "ninja not found."

# -------------------------
# Optionally clean build dir
# -------------------------
if [[ "${CLEAN}" == true ]]; then
  warn "Cleaning build directory: ${BUILD_DIR}"
  rm -rf "${BUILD_DIR}"
fi

# -------------------------
# Determine whether build is configured and whether it uses Clang
# -------------------------
is_configured() {
  [[ -f "${BUILD_DIR}/build.ninja" && -f "${BUILD_DIR}/CMakeCache.txt" ]]
}

configured_compiler_is_clang() {
  # Returns 0 if CMakeCache reports Clang/clang++ paths matching our chosen clang
  [[ -f "${BUILD_DIR}/CMakeCache.txt" ]] || return 1

  local cache_cc cache_cxx
  cache_cc="$(grep -E '^CMAKE_C_COMPILER:FILEPATH=' "${BUILD_DIR}/CMakeCache.txt" | head -n 1 | cut -d= -f2- || true)"
  cache_cxx="$(grep -E '^CMAKE_CXX_COMPILER:FILEPATH=' "${BUILD_DIR}/CMakeCache.txt" | head -n 1 | cut -d= -f2- || true)"

  [[ "${cache_cc}" == "${CLANG_C}" && "${cache_cxx}" == "${CLANG_CXX}" ]]
}

# -------------------------
# Configure step (forces Clang)
# -------------------------
configure_with_clang() {
  info "Configuring with Clang:"
  info "  CC:  ${CLANG_C}"
  info "  CXX: ${CLANG_CXX}"

  mkdir -p "${BUILD_DIR}"

  cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" -G Ninja \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_C_COMPILER="${CLANG_C}" \
    -DCMAKE_CXX_COMPILER="${CLANG_CXX}"
}

# -------------------------
# Ensure configured correctly
# -------------------------
if ! is_configured; then
  warn "Build not configured. Running setup (CMake) with Clang..."
  "${SCRIPTS_DIR}/setup.sh" --use-clang || die "Setup failed."
fi

# After setup, verify compiler. If mismatch, reconfigure robustly.
if [[ "${FORCE_RECONFIGURE}" == true ]]; then
  warn "Forcing reconfigure with Clang (--reconfigure)."
  configure_with_clang
else
  if configured_compiler_is_clang; then
    info "CMake already configured with the desired Clang. ✓"
  else
    warn "CMake is configured with a different compiler. Recreating build dir to force Clang..."
    rm -rf "${BUILD_DIR}"
    configure_with_clang
  fi
fi

# -------------------------
# Build
# -------------------------
echo -e "${CYAN}=== Filament Engine — Build (${JOBS} jobs) ===${NC}"
ninja -C "${BUILD_DIR}" -j"${JOBS}" "${EXTRA_NINJA_ARGS[@]}"

echo ""
echo "✅ Build complete. Binary at: ${BUILD_DIR}/sandbox/sandbox"
