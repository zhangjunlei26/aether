$env:PATH = "C:\cygwin64\bin;$env:PATH"

$examples = @(
    "tests\test_type_inference_literals.ae",
    "examples\basic\ultra_simple.ae",
    "examples\basic\hello_world.ae",
    "tests\test_type_inference_functions.ae",
    "tests\test_type_inference_structs.ae"
)

$passed = 0
$failed = 0

foreach ($example in $examples) {
    $name = [System.IO.Path]::GetFileNameWithoutExtension($example)
    $output_c = "build\$name.c"
    $output_exe = "build\$name.exe"
    
    Write-Host "`n========================================" -ForegroundColor Cyan
    Write-Host "Testing: $example" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    
    # Compile Aether to C
    .\build\aetherc.exe $example $output_c 2>&1 | Out-Null
    if ($LASTEXITCODE -ne 0) {
        Write-Host "❌ FAILED: Aether compilation" -ForegroundColor Red
        $failed++
        continue
    }
    
    # Compile C to executable
    gcc $output_c -Iruntime runtime\multicore_scheduler.c runtime\memory.c runtime\aether_string.c -o $output_exe 2>&1 | Out-Null
    if ($LASTEXITCODE -ne 0) {
        Write-Host "❌ FAILED: C compilation" -ForegroundColor Red
        $failed++
        continue
    }
    
    # Run
    & $output_exe 2>&1 | Out-Null
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✅ PASSED" -ForegroundColor Green
        $passed++
    } else {
        Write-Host "❌ FAILED: Runtime error" -ForegroundColor Red
        $failed++
    }
}

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "Results: $passed passed, $failed failed" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

