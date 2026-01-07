# Aether Standard Library Test Runner (Windows/PowerShell)

# Change to project root directory (tests/stdlib -> project root)
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Split-Path -Parent (Split-Path -Parent $scriptDir)
Push-Location $projectRoot

Write-Host "`n==================================" -ForegroundColor Cyan
Write-Host "  Aether Standard Library Tests" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan

$passed = 0
$failed = 0

# Compiler flags
$cflags = "-I. -Istd -Iruntime -Itests/stdlib -std=c11 -Wall -Wextra"

# Test Vector
Write-Host "`nCompiling Vector tests..." -ForegroundColor Yellow
$output = gcc -o test_vector_run.exe tests/stdlib/test_vector.c std/collections/aether_vector.c runtime/memory.c $cflags.Split() 2>&1
if ($LASTEXITCODE -eq 0) {
    Write-Host "Running Vector tests..." -ForegroundColor Yellow
    .\test_vector_run.exe
    if ($LASTEXITCODE -eq 0) { $passed++ } else { $failed++ }
} else {
    Write-Host "FAILED to compile Vector tests" -ForegroundColor Red
    $failed++
}

# Test HashMap
Write-Host "`nCompiling HashMap tests..." -ForegroundColor Yellow
$output = gcc -o test_hashmap_run.exe tests/stdlib/test_hashmap.c std/collections/aether_hashmap.c runtime/memory.c $cflags.Split() 2>&1
if ($LASTEXITCODE -eq 0) {
    Write-Host "Running HashMap tests..." -ForegroundColor Yellow
    .\test_hashmap_run.exe
    if ($LASTEXITCODE -eq 0) { $passed++ } else { $failed++ }
} else {
    Write-Host "FAILED to compile HashMap tests" -ForegroundColor Red
    $failed++
}

# Test Set
Write-Host "`nCompiling Set tests..." -ForegroundColor Yellow
$output = gcc -o test_set_run.exe tests/stdlib/test_set.c std/collections/aether_set.c std/collections/aether_hashmap.c runtime/memory.c $cflags.Split() 2>&1
if ($LASTEXITCODE -eq 0) {
    Write-Host "Running Set tests..." -ForegroundColor Yellow
    .\test_set_run.exe
    if ($LASTEXITCODE -eq 0) { $passed++ } else { $failed++ }
} else {
    Write-Host "FAILED to compile Set tests" -ForegroundColor Red
    $failed++
}

# Summary
Write-Host "`n==================================" -ForegroundColor Cyan
Write-Host "  Test Summary" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host "Passed: $passed" -ForegroundColor Green
$failColor = if ($failed -eq 0) { "Green" } else { "Red" }
Write-Host "Failed: $failed" -ForegroundColor $failColor
Write-Host "==================================" -ForegroundColor Cyan

# Cleanup
Remove-Item -Path test_vector_run.exe, test_hashmap_run.exe, test_set_run.exe -ErrorAction SilentlyContinue

# Return to original directory
Pop-Location

if ($failed -eq 0) {
    Write-Host "`nAll tests passed!" -ForegroundColor Green
    exit 0
} else {
    Write-Host "`nSome tests failed." -ForegroundColor Red
    exit 1
}
