# Aether Compiler Tests Runner
# Compiles ALL compiler tests together with test harness

$scriptDir = $PSScriptRoot
$rootDir = (Get-Item $scriptDir).Parent.Parent.FullName

Write-Host ""
Write-Host "==================================" -ForegroundColor Cyan
Write-Host "  Aether Compiler Tests" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host ""

# All compiler test files (except test_arrays which has its own main)
$testFiles = @(
    "tests/compiler/test_lexer.c",
    "tests/compiler/test_lexer_comprehensive.c",
    "tests/compiler/test_parser.c",
    "tests/compiler/test_parser_comprehensive.c",
    "tests/compiler/test_typechecker.c",
    "tests/compiler/test_type_inference_comprehensive.c",
    "tests/compiler/test_switch_statements.c",
    "tests/compiler/test_pattern_matching_comprehensive.c",
    "tests/compiler/test_codegen.c",
    "tests/compiler/test_structs.c"
)

# Test harness and main
$frameworkFiles = @(
    "tests/compiler/test_harness.c",
    "tests/compiler/test_runner_main.c"
)

# Compiler dependencies
$compilerDeps = @(
    "compiler/lexer.c",
    "compiler/parser.c",
    "compiler/typechecker.c",
    "compiler/type_inference.c",
    "compiler/codegen.c",
    "compiler/ast.c",
    "compiler/aether_error.c",
    "runtime/memory.c"
)

$allSources = $testFiles + $frameworkFiles + $compilerDeps
$includes = "-I. -Icompiler -Iruntime -Itests/compiler"
$flags = "-std=c11"
$output = "tests/compiler/compiler_tests_all.exe"

Write-Host "Compiling all compiler tests together..."
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
        $mainTestsResult = $LASTEXITCODE
        
        # Compile and run test_arrays separately (it has its own main)
        Write-Host ""
        Write-Host "Compiling test_arrays (standalone)..."
        $arraysCmd = "gcc tests/compiler/test_arrays.c $($compilerDeps -join ' ') -o tests/compiler/test_arrays.exe $includes $flags"
        Invoke-Expression $arraysCmd *>&1 | Out-Null
        
        if (Test-Path "tests/compiler/test_arrays.exe") {
            & ".\tests\compiler\test_arrays.exe"
            $arraysResult = $LASTEXITCODE
        } else {
            Write-Host "test_arrays compilation failed!" -ForegroundColor Red
            $arraysResult = 1
        }
        
        if ($mainTestsResult -eq 0 -and $arraysResult -eq 0) {
            Write-Host ""
            Write-Host "==================================" -ForegroundColor Cyan
            Write-Host "  Test Summary" -ForegroundColor Cyan
            Write-Host "==================================" -ForegroundColor Cyan
            Write-Host "All compiler tests passed!" -ForegroundColor Green
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
