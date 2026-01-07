# Aether Memory Tests Runner
# Compiles ALL memory tests together with test harness

$scriptDir = $PSScriptRoot
$rootDir = (Get-Item $scriptDir).Parent.Parent.FullName

Write-Host ""
Write-Host "==================================" -ForegroundColor Cyan
Write-Host "  Aether Memory Tests" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host ""

# All memory test files
$testFiles = @(
    "tests/memory/test_memory_arena.c",
    "tests/memory/test_memory_pool.c",
    "tests/memory/test_memory_leaks.c",
    "tests/memory/test_memory_stress.c"
)

# Test harness and main
$frameworkFiles = @(
    "tests/memory/test_harness.c",
    "tests/memory/test_runner_main.c"
)

# Memory system dependencies
$memoryDeps = @(
    "runtime/memory.c",
    "runtime/aether_arena.c",
    "runtime/aether_pool.c",
    "runtime/aether_memory_stats.c"
)

$allSources = $testFiles + $frameworkFiles + $memoryDeps
$includes = "-I. -Iruntime -Itests/memory"
$flags = "-std=c11"
$output = "tests/memory/memory_tests_all.exe"

Write-Host "Compiling all memory tests together..."
$compileCmd = "gcc $($allSources -join ' ') -o $output $includes $flags"

Push-Location $rootDir
try {
    # Run compilation silently
    $ErrorActionPreference = "Continue"
    Invoke-Expression $compileCmd *>&1 | Out-Null
    
    if (Test-Path $output) {
        Write-Host "Compilation successful!" -ForegroundColor Green
        Write-Host ""
        
        # Run all tests
        & ".\$output"
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host ""
            Write-Host "==================================" -ForegroundColor Cyan
            Write-Host "  Test Summary" -ForegroundColor Cyan
            Write-Host "==================================" -ForegroundColor Cyan
            Write-Host "All memory tests passed!" -ForegroundColor Green
            Write-Host "==================================" -ForegroundColor Cyan
            exit 0
        } else {
            Write-Host ""
            Write-Host "Some tests failed." -ForegroundColor Red
            exit 1
        }
    } else {
        Write-Host "Compilation failed!" -ForegroundColor Red
        exit 1
    }
}
finally {
    Pop-Location
}
