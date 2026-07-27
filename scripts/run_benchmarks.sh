#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build-release"

echo "Building Release benchmarks..."
cmake -DCMAKE_BUILD_TYPE=Release -B "${BUILD_DIR}" -S .
cmake --build "${BUILD_DIR}" --target bench_skip_list -j4

echo ""
echo "Running SkipList benchmark..."
"${BUILD_DIR}/benchmarks/bench_skip_list"
