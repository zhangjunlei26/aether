#!/bin/bash
# Cross-platform build script for Aether compiler
# Works on Linux, macOS, and Git Bash/WSL on Windows

set -e  # Exit on error

echo "========================================="
echo "Building Aether Compiler"
echo "========================================="

# Create build directory
mkdir -p build

# Compiler sources
COMPILER_SRC="compiler/aetherc.c compiler/lexer.c compiler/parser.c compiler/ast.c compiler/typechecker.c compiler/codegen.c"

# Build compiler
echo "Compiling..."
gcc -O2 -Icompiler -Iruntime -Wall $COMPILER_SRC -o build/aetherc

echo ""
echo "✓ Build successful!"
echo "Compiler: build/aetherc"

