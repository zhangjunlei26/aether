# Aether Runtime Tests Runner
# Compiles ALL runtime tests together with test harness

$ErrorActionPreference = "Stop"
$scriptDir = $PSScriptRoot
$rootDir = (Get-Item $scriptDir).Parent.Parent.FullName

Write-Host ""
Write-Host "==================================" -ForegroundColor Cyan
Write-Host "  Aether Runtime Tests" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host ""

# All runtime test files - compile together
$testFiles = @(
    "tests/runtime/test_64bit.c",
    "tests/runtime/test_runtime_collections.c",
    "tests/runtime/test_runtime_http.c",
    "tests/runtime/test_runtime_json.c",
    "tests/runtime/test_runtime_math.c",
    "tests/runtime/test_runtime_net.c",
    "tests/runtime/test_runtime_strings.c"
)

# Test harness and main
$frameworkFiles = @(
    "tests/runtime/test_harness.c",
    "tests/runtime/test_runner_main.c"
)

# All runtime library dependencies
$runtimeDeps = @(
    "runtime/memory.c",
    "std/collections/aether_vector.c",
    "std/collections/aether_hashmap.c",
    "std/collections/aether_set.c",
    "std/collections/aether_collections.c",
    "std/string/aether_string.c",
    "std/json/aether_json.c",
    "std/math/aether_math.c",
    "std/net/aether_http.c",
    "std/net/aether_http_server.c",
    "std/net/aether_net.c"
)

$allSources = $testFiles + $frameworkFiles + $runtimeDeps
$includes = "-I. -Istd -Iruntime -Itests/runtime"
$flags = "-std=c11 -Wall -Wextra"
$linkerFlags = "-lws2_32"  # Windows network library
$output = "tests/runtime/runtime_tests_all.exe"

Write-Host "Compiling all runtime tests together..."
$compileCmd = "gcc $($allSources -join ' ') -o $output $includes $flags $linkerFlags"

Push-Location $rootDir
try {
    # Run compilation silently (warnings go to stderr but that's OK)
    $ErrorActionPreference = "Continue"  # Don't treat stderr as error
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
            Write-Host "All runtime tests passed!" -ForegroundColor Green
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
