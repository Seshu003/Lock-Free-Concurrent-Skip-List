#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build-release"

echo "Building Release tests..."
cmake -DCMAKE_BUILD_TYPE=Release -B "${BUILD_DIR}" -S .
cmake --build "${BUILD_DIR}" -j4

echo ""
echo "Running tests..."
"${BUILD_DIR}/tests/test_skip_list_basic"
"${BUILD_DIR}/tests/test_hazard_pointers"
"${BUILD_DIR}/tests/test_concurrent_stress"

echo "All tests passed successfully."
