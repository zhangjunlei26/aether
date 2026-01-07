# Aether Master Test Runner (Windows/PowerShell)
# Runs ALL test categories in the Aether test suite

param(
    [switch]$StdlibOnly,
    [switch]$RuntimeOnly,
    [switch]$CompilerOnly,
    [switch]$MemoryOnly,
    [switch]$Verbose
)

$ErrorActionPreference = "Continue"

Write-Host "`n============================================" -ForegroundColor Cyan
Write-Host "  Aether Master Test Suite" -ForegroundColor Cyan
Write-Host "  Professional C Testing System" -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan

$categories_passed = 0
$categories_failed = 0
$test_results = @()

# [1/5] Standard Library Tests
if (-not $RuntimeOnly -and -not $CompilerOnly -and -not $MemoryOnly) {
    Write-Host "`n[1/5] Running Standard Library Tests..." -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    
    if (Test-Path "stdlib\run_tests.ps1") {
        & "stdlib\run_tests.ps1"
        if ($LASTEXITCODE -eq 0) {
            Write-Host "[PASS] Standard Library" -ForegroundColor Green
            $categories_passed++
            $test_results += "[PASS] stdlib"
        } else {
            Write-Host "[FAIL] Standard Library" -ForegroundColor Red
            $categories_failed++
            $test_results += "[FAIL] stdlib"
        }
    } else {
        Write-Host "[ERROR] stdlib runner not found" -ForegroundColor Red
        $categories_failed++
        $test_results += "[ERROR] stdlib"
    }
}

# [2/5] Runtime Tests
if (-not $StdlibOnly -and -not $CompilerOnly -and -not $MemoryOnly) {
    Write-Host "`n[2/5] Running Runtime Tests..." -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    
    if (Test-Path "runtime\run_tests.ps1") {
        & "runtime\run_tests.ps1"
        if ($LASTEXITCODE -eq 0) {
            Write-Host "[PASS] Runtime Tests" -ForegroundColor Green
            $categories_passed++
            $test_results += "[PASS] runtime"
        } else {
            Write-Host "[FAIL] Runtime Tests" -ForegroundColor Red
            $categories_failed++
            $test_results += "[FAIL] runtime"
        }
    } else {
        Write-Host "[ERROR] runtime runner not found" -ForegroundColor Red
        $categories_failed++
        $test_results += "[ERROR] runtime"
    }
}

# [3/5] Compiler Tests
if (-not $StdlibOnly -and -not $RuntimeOnly -and -not $MemoryOnly) {
    Write-Host "`n[3/5] Running Compiler Tests..." -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    
    if (Test-Path "compiler\run_tests.ps1") {
        & "compiler\run_tests.ps1"
        if ($LASTEXITCODE -eq 0) {
            Write-Host "[PASS] Compiler Tests" -ForegroundColor Green
            $categories_passed++
            $test_results += "[PASS] compiler"
        } else {
            Write-Host "[FAIL] Compiler Tests" -ForegroundColor Red
            $categories_failed++
            $test_results += "[FAIL] compiler"
        }
    } else {
        Write-Host "[ERROR] compiler runner not found" -ForegroundColor Red
        $categories_failed++
        $test_results += "[ERROR] compiler"
    }
}

# [4/5] Memory Tests
if (-not $StdlibOnly -and -not $RuntimeOnly -and -not $CompilerOnly) {
    Write-Host "`n[4/5] Running Memory Tests..." -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    
    if (Test-Path "memory\run_tests.ps1") {
        & "memory\run_tests.ps1"
        if ($LASTEXITCODE -eq 0) {
            Write-Host "[PASS] Memory Tests" -ForegroundColor Green
            $categories_passed++
            $test_results += "[PASS] memory"
        } else {
            Write-Host "[FAIL] Memory Tests" -ForegroundColor Red
            $categories_failed++
            $test_results += "[FAIL] memory"
        }
    } else {
        Write-Host "[ERROR] memory runner not found" -ForegroundColor Red
        $categories_failed++
        $test_results += "[ERROR] memory"
    }
}

# [5/5] Integration Tests
if (-not $StdlibOnly -and -not $RuntimeOnly -and -not $CompilerOnly -and -not $MemoryOnly) {
    Write-Host "`n[5/5] Integration Tests..." -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "Note: Integration tests require manual execution" -ForegroundColor Yellow
    Write-Host "See tests/integration/ for test files" -ForegroundColor Yellow
    $test_results += "[SKIP] integration"
}

# Final Summary
Write-Host "`n============================================" -ForegroundColor Cyan
Write-Host "  Test Summary" -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan

foreach ($result in $test_results) {
    if ($result -match "\[PASS\]") {
        Write-Host $result -ForegroundColor Green
    } elseif ($result -match "\[FAIL\]") {
        Write-Host $result -ForegroundColor Red
    } elseif ($result -match "\[ERROR\]") {
        Write-Host $result -ForegroundColor Red
    } elseif ($result -match "\[SKIP\]") {
        Write-Host $result -ForegroundColor Yellow
    }
}

Write-Host "`nCategories Passed: $categories_passed" -ForegroundColor Green
Write-Host "Categories Failed: $categories_failed" -ForegroundColor $(if ($categories_failed -eq 0) { "Green" } else { "Red" })
Write-Host "============================================" -ForegroundColor Cyan

if ($categories_failed -eq 0) {
    Write-Host "`nAll tested categories passed!" -ForegroundColor Green
    exit 0
} else {
    Write-Host "`nSome categories failed." -ForegroundColor Red
    exit 1
}
