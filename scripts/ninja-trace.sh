#!/bin/bash
# Ninja build trace visualization script
#
# Converts the Ninja build log to Chrome tracing format for visualization.
# This helps identify parallelism bottlenecks and the critical path.
#
# Prerequisites:
#   - ninjatracing: pip install ninjatracing
#   - A completed build (ninja log must exist)
#
# Usage:
#   ./scripts/ninja-trace.sh [preset]
#   ./scripts/ninja-trace.sh dev-gcc
#
# Output:
#   Creates trace.json in the build directory.
#   Open in Chrome at chrome://tracing

set -e

PRESET="${1:-dev-gcc}"
BUILD_DIR="build/${PRESET}"
NINJA_LOG="${BUILD_DIR}/.ninja_log"
TRACE_FILE="${BUILD_DIR}/trace.json"

# Check for ninjatracing
if ! command -v ninjatracing &> /dev/null; then
    echo "Error: ninjatracing not found"
    echo "Install with: pip install ninjatracing"
    exit 1
fi

# Check for ninja log
if [[ ! -f "${NINJA_LOG}" ]]; then
    echo "Error: Ninja log not found at ${NINJA_LOG}"
    echo "Run a build first: cmake --build --preset ${PRESET}"
    exit 1
fi

echo "=== Ninja Build Trace ==="
echo "Preset: ${PRESET}"
echo ""

# Generate trace
echo "Converting ninja log to Chrome trace format..."
ninjatracing "${NINJA_LOG}" > "${TRACE_FILE}"

echo ""
echo "Trace file created: ${TRACE_FILE}"
echo ""
echo "To view:"
echo "  1. Open Chrome browser"
echo "  2. Navigate to chrome://tracing"
echo "  3. Click 'Load' and select: ${TRACE_FILE}"
echo ""
echo "What to look for:"
echo "  - Long bars = slow compilation units"
echo "  - Gaps = parallelism bottlenecks"
echo "  - Critical path = longest chain of dependent tasks"
