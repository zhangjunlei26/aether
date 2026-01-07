#!/bin/bash
# Profile-Guided Optimization (PGO) Build Script
# This script performs a 3-stage build for maximum performance

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "=== Aether PGO Build Pipeline ==="
echo ""

# Stage 1: Build with profiling instrumentation
echo "[Stage 1] Building with profiling instrumentation..."
cd "$PROJECT_ROOT"

gcc compiler/*.c runtime/aether_message_registry.c runtime/aether_actor_thread.c \
    -I runtime \
    -o aetherc_profgen \
    -O3 -march=native -fprofile-generate \
    -Wall -Wno-unused-variable -Wno-unused-function

echo "  ✓ Profiling build complete"

# Stage 2: Run benchmarks to collect profile data
echo ""
echo "[Stage 2] Collecting profile data..."

# Compile and run representative workloads
echo "  → Compiling benchmark programs..."

# Build runtime benchmark
gcc bench_runtime.c -I runtime -o bench_profile \
    -O3 -march=native -fprofile-generate

# Run benchmarks (generates .gcda files)
echo "  → Running mailbox benchmark..."
./bench_profile > /dev/null

echo "  ✓ Profile data collected"

# Stage 3: Rebuild with profile data
echo ""
echo "[Stage 3] Building optimized binary with PGO..."

gcc compiler/*.c runtime/aether_message_registry.c runtime/aether_actor_thread.c \
    -I runtime \
    -o aetherc.exe \
    -O3 -march=native -fprofile-use -flto \
    -Wall -Wno-unused-variable -Wno-unused-function

echo "  ✓ PGO build complete"

# Cleanup
echo ""
echo "[Cleanup] Removing profile data..."
rm -f *.gcda *.gcno
rm -f bench_profile aetherc_profgen

echo ""
echo "=== PGO Build Complete ==="
echo ""
echo "Expected improvements:"
echo "  • 10-20% better performance from optimal inlining"
echo "  • Better branch prediction"
echo "  • Improved code layout (hot paths together)"
echo ""
echo "Compiler binary: aetherc.exe"
