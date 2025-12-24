#!/bin/bash
# Cross-platform test script for Aether
# Works on Linux, macOS, and Git Bash/WSL on Windows

set -e  # Exit on error

echo "========================================="
echo "Building and Testing Aether"
echo "========================================="
echo ""

# Build compiler first
./build.sh

echo ""
echo "========================================="
echo "Building Test Suite"
echo "========================================="

# Collect test files (exclude manual tests)
TEST_SRC=$(ls tests/test_*.c | grep -v test_runtime_manual.c | tr '\n' ' ')
COMPILER_SRC="compiler/aetherc.c compiler/lexer.c compiler/parser.c compiler/ast.c compiler/typechecker.c compiler/codegen.c"

gcc -O2 -Icompiler -Iruntime -Wall $TEST_SRC $COMPILER_SRC -o build/test_runner

echo ""
echo "========================================="
echo "Running Test Suite"
echo "========================================="
./build/test_runner

echo ""
echo "========================================="
echo "✓ All Tests Passed!"
echo "========================================="

