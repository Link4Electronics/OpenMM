#!/usr/bin/env bash
# reccmp verification workflow for OpenMM (modeled on dethrace's reccmp-native.sh).
#
# Compares the C++ reimplementation, compiled with a period-correct MSVC
# toolchain under Wine, against the original Midtown Madness beta binary.
# Functions must be annotated with `// FUNCTION: MIDTOWN 0x...` comments
# (see tools/map_annotate.py) for reccmp to pair them up.
#
# Requires:
#   - Wine prefix with MSVC (VC6-era for the 1999 beta) at WINEPREFIX
#   - Original midtown.exe in the repo root (see reccmp-project.yml)
#   - reccmp pip package: pip install reccmp
#
# Usage:
#   tools/reccmp.sh               # configure + build + full report
#   tools/reccmp.sh --config      # just (re)configure the MSVC build dir
#   tools/reccmp.sh --build       # just rebuild with MSVC
#   tools/reccmp.sh --report      # full html/json report (no build)
#   tools/reccmp.sh 0x401030      # build + diff a single function
#   tools/reccmp.sh --help        # this message
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${OPENMM_RECCMP_BUILD_DIR:-${PROJECT_DIR}/build_msvc}"
WINEPREFIX="${WINEPREFIX:-${PROJECT_DIR}/../openmm-wineprefix}"
RECCMP_VENV="${RECCMP_VENV:-${TMPDIR:-/tmp}/reccmp-venv}"
ORIGINAL="${PROJECT_DIR}/midtown.exe"
TARGET="MIDTOWN"
RECCMP="${RECCMP_VENV}/bin/reccmp-reccmp"

export WINEPREFIX
export WINEDEBUG=-all
export WINEDLLOVERRIDES=mscoree,mshtml=
export PATH="${RECCMP_VENV}/bin:$PATH"

[ -f "$ORIGINAL" ] || {
    echo "ERROR: $ORIGINAL not found."
    echo "Place the original beta executable in the repo root as midtown.exe"
    echo "(or edit reccmp-user.yml), then run: reccmp-project detect --search-path $PROJECT_DIR"
    exit 1
}
[ -x "$RECCMP" ] || {
    echo "ERROR: reccmp not found at $RECCMP"
    echo "  python3 -m venv $RECCMP_VENV && $RECCMP_VENV/bin/pip install reccmp"
    exit 1
}

configure() {
    echo "==> Configuring MSVC build via wine ..."
    (cd "$PROJECT_DIR" && wine cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Debug -DOPENMM_RECCMP=ON)
}

build() {
    echo "==> Building with MSVC ..."
    (cd "$PROJECT_DIR" && wine cmake --build "$BUILD_DIR" -- -j1 2>&1 | tail -5)
    # CMake writes the build config with Wine Z:/ paths; strip them so reccmp
    # can resolve the files on the host (same trick dethrace uses).
    sed -i 's/Z://g' "$BUILD_DIR/reccmp-build.yml"
    sed -i 's/midtown\.exe/midtown.exe/g; s/MIDTOWN\.exe/midtown.exe/g' "$BUILD_DIR/reccmp-build.yml"
}

diff_function() {
    local addr="$1"
    echo "==> Diffing function at $addr ..."
    (cd "$BUILD_DIR" && "$RECCMP" --target "$TARGET" --no-color --verbose "$addr" 2>&1 \
        | grep -v "Failed to match variable\|No function for static\|No function for\|Failed to find function symbol\|Failed to match function\|Parsing\|fixme\|Dropped\|larger in recomp")
}

full_report() {
    echo "==> Generating full report ..."
    (cd "$BUILD_DIR" && "$RECCMP" --target "$TARGET" --silent --json "$BUILD_DIR/reccmp-report.json" --html "$BUILD_DIR/reccmp-report.html" 2>&1 \
        | grep -v "Failed to match variable\|No function for static\|No function for\|Failed to find function symbol\|Failed to match function\|Parsing\|fixme\|Dropped\|larger in recomp")
    echo "==> Report: $BUILD_DIR/reccmp-report.html"
}

case "${1:-}" in
    --config) configure ;;
    --build) build ;;
    --report) full_report ;;
    --help|-h) sed -n '2,26p' "$0" ;;
    "")
        [ -f "$BUILD_DIR/CMakeCache.txt" ] || configure
        build
        full_report
        ;;
    *)
        build
        diff_function "$1"
        ;;
esac
