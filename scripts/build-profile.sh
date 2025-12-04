#!/bin/bash
# Build profiling script using ClangBuildAnalyzer
#
# This script profiles the build to identify slow compilation units,
# expensive templates, and header dependencies.
#
# Prerequisites:
#   - ClangBuildAnalyzer: https://github.com/aras-p/ClangBuildAnalyzer
#   - Clang compiler (uses -ftime-trace flag)
#
# Usage:
#   ./scripts/build-profile.sh [preset]
#   ./scripts/build-profile.sh dev-clang
#
# Output:
#   Prints analysis to stdout showing:
#   - Slowest files to compile
#   - Most expensive template instantiations
#   - Header files included most often

set -e

PRESET="${1:-dev-clang}"
BUILD_DIR="build/${PRESET}"
CAPTURE_FILE="${BUILD_DIR}/capture.bin"

# Check for ClangBuildAnalyzer
if ! command -v ClangBuildAnalyzer &> /dev/null; then
    echo "Error: ClangBuildAnalyzer not found"
    echo "Install from: https://github.com/aras-p/ClangBuildAnalyzer"
    echo ""
    echo "On Ubuntu/Debian:"
    echo "  git clone https://github.com/aras-p/ClangBuildAnalyzer"
    echo "  cd ClangBuildAnalyzer && mkdir build && cd build"
    echo "  cmake .. && make && sudo make install"
    exit 1
fi

# Must use Clang for -ftime-trace support
if [[ "${PRESET}" != *"clang"* ]]; then
    echo "Warning: -ftime-trace requires Clang. Switching to dev-clang preset."
    PRESET="dev-clang"
    BUILD_DIR="build/${PRESET}"
    CAPTURE_FILE="${BUILD_DIR}/capture.bin"
fi

echo "=== Build Profiling with ClangBuildAnalyzer ==="
echo "Preset: ${PRESET}"
echo ""

# Clean and configure with time-trace enabled
echo "Cleaning previous build..."
rm -rf "${BUILD_DIR}"

echo "Configuring with -ftime-trace..."
cmake --preset "${PRESET}" \
    -DCMAKE_CXX_FLAGS="-ftime-trace"

# Start capturing
echo "Starting capture..."
ClangBuildAnalyzer --start "${BUILD_DIR}"

# Build
echo "Building..."
cmake --build --preset "${PRESET}"

# Stop capturing
echo "Stopping capture..."
ClangBuildAnalyzer --stop "${BUILD_DIR}" "${CAPTURE_FILE}"

# Analyze
echo ""
echo "=== Analysis Results ==="
echo ""
ClangBuildAnalyzer --analyze "${CAPTURE_FILE}"

echo ""
echo "Capture file saved to: ${CAPTURE_FILE}"
echo "Re-analyze with: ClangBuildAnalyzer --analyze ${CAPTURE_FILE}"
