# Profile-Guided Optimization (PGO) Build Script for Windows
# PowerShell version

$ErrorActionPreference = "Stop"

Write-Host "=== Aether PGO Build Pipeline ===" -ForegroundColor Cyan
Write-Host ""

# Stage 1: Build with profiling instrumentation
Write-Host "[Stage 1] Building with profiling instrumentation..." -ForegroundColor Yellow

gcc compiler/*.c runtime/aether_message_registry.c runtime/aether_actor_thread.c `
    -I runtime `
    -o aetherc_profgen.exe `
    -O3 -march=native -fprofile-generate `
    -Wall -Wno-unused-variable -Wno-unused-function 2>&1 | Out-Null

if ($LASTEXITCODE -eq 0) {
    Write-Host "  ✓ Profiling build complete" -ForegroundColor Green
} else {
    Write-Host "  ✗ Build failed" -ForegroundColor Red
    exit 1
}

# Stage 2: Run benchmarks to collect profile data
Write-Host ""
Write-Host "[Stage 2] Collecting profile data..." -ForegroundColor Yellow

# Build runtime benchmark
gcc bench_runtime.c -I runtime -o bench_profile.exe `
    -O3 -march=native -fprofile-generate 2>&1 | Out-Null

# Run benchmarks (generates .gcda files)
Write-Host "  -> Running mailbox benchmark..."
.\bench_profile.exe | Out-Null

Write-Host "  ✓ Profile data collected" -ForegroundColor Green

# Stage 3: Rebuild with profile data
Write-Host ""
Write-Host "[Stage 3] Building optimized binary with PGO..." -ForegroundColor Yellow

gcc compiler/*.c runtime/aether_message_registry.c runtime/aether_actor_thread.c `
    -I runtime `
    -o aetherc.exe `
    -O3 -march=native -fprofile-use `
    -Wall -Wno-unused-variable -Wno-unused-function 2>&1 | Out-Null

if ($LASTEXITCODE -eq 0) {
    Write-Host "  ✓ PGO build complete" -ForegroundColor Green
} else {
    Write-Host "  ✗ PGO build failed" -ForegroundColor Red
    exit 1
}

# Cleanup
Write-Host ""
Write-Host "[Cleanup] Removing profile data..." -ForegroundColor Yellow
Remove-Item -Path "*.gcda" -ErrorAction SilentlyContinue
Remove-Item -Path "*.gcno" -ErrorAction SilentlyContinue
Remove-Item -Path "bench_profile.exe" -ErrorAction SilentlyContinue
Remove-Item -Path "aetherc_profgen.exe" -ErrorAction SilentlyContinue

Write-Host ""
Write-Host "=== PGO Build Complete ===" -ForegroundColor Cyan
Write-Host ""
Write-Host "Expected improvements:" -ForegroundColor White
Write-Host "  • 10-20% better performance from optimal inlining"
Write-Host "  • Better branch prediction"
Write-Host "  • Improved code layout (hot paths together)"
Write-Host ""
Write-Host "Compiler binary: aetherc.exe" -ForegroundColor Green
