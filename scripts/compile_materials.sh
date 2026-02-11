#!/usr/bin/env bash
# ==============================================================================
# Compile Filament materials (.mat → .filamat)
#
# Usage:
#   ./scripts/compile_materials.sh                  # Compile all
#   ./scripts/compile_materials.sh standard_lit     # Compile specific material
# ==============================================================================
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MATC="${PROJECT_ROOT}/vendor/filament/out/release/filament/bin/matc"
MATERIAL_DIR="${PROJECT_ROOT}/materials"
OUTPUT_DIR="${PROJECT_ROOT}/build/sandbox/materials"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
CYAN='\033[0;36m'
NC='\033[0m'

if [ ! -x "${MATC}" ]; then
    echo -e "${RED}[ERROR]${NC} matc not found at: ${MATC}"
    echo "Run ./scripts/setup.sh first."
    exit 1
fi

mkdir -p "${OUTPUT_DIR}"

# Detect platform-appropriate API flags
if [ "$(uname -s)" = "Darwin" ]; then
    API_FLAGS="-a metal -a opengl"
else
    API_FLAGS="-a vulkan -a opengl"
fi

compile_material() {
    local mat_file="$1"
    local name
    name="$(basename "${mat_file}" .mat)"
    local output="${OUTPUT_DIR}/${name}.filamat"

    echo -e "${CYAN}[MATC]${NC} ${name}.mat → ${name}.filamat"
    "${MATC}" ${API_FLAGS} -o "${output}" "${mat_file}"
    echo -e "${GREEN}  ✓${NC} ${output}"
}

if [ $# -gt 0 ]; then
    # Compile specific material(s)
    for name in "$@"; do
        mat_file="${MATERIAL_DIR}/${name}.mat"
        if [ ! -f "${mat_file}" ]; then
            echo -e "${RED}[ERROR]${NC} Material not found: ${mat_file}"
            exit 1
        fi
        compile_material "${mat_file}"
    done
else
    # Compile all materials
    count=0
    for mat_file in "${MATERIAL_DIR}"/*.mat; do
        [ -f "${mat_file}" ] || continue
        compile_material "${mat_file}"
        ((count++))
    done

    if [ ${count} -eq 0 ]; then
        echo -e "${RED}[WARN]${NC} No .mat files found in ${MATERIAL_DIR}"
    else
        echo ""
        echo -e "${GREEN}[DONE]${NC} ${count} material(s) compiled."
    fi
fi
