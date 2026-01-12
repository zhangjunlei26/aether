# Cross-Language Benchmark Runner for Windows
# Compiles and runs benchmarks, generates results.json

$ErrorActionPreference = "Stop"

Write-Host "=" -NoNewline; Write-Host ("=" * 59)
Write-Host "  Cross-Language Actor Benchmark Suite"
Write-Host "=" -NoNewline; Write-Host ("=" * 59)

# Initialize results
$results = @{
    timestamp = (Get-Date).ToUniversalTime().ToString("yyyy-MM-ddTHH:mm:ssZ")
    hardware = @{
        cpu = (Get-WmiObject Win32_Processor).Name
        cores = (Get-WmiObject Win32_Processor).NumberOfLogicalProcessors
        os = "Windows"
    }
    benchmarks = @{}
}

Write-Host "CPU:   $($results.hardware.cpu)"
Write-Host "Cores: $($results.hardware.cores)"
Write-Host "OS:    $($results.hardware.os)"
Write-Host "=" -NoNewline; Write-Host ("=" * 59)

function Run-Benchmark {
    param($Name, $Command, $BuildCommand = $null)
    
    Write-Host "`n=== Benchmarking $Name ===" -ForegroundColor Cyan
    
    if ($BuildCommand) {
        Write-Host "Building..."
        Invoke-Expression $BuildCommand 2>&1 | Out-Null
        if ($LASTEXITCODE -ne 0) {
            Write-Host "X Build failed" -ForegroundColor Red
            return
        }
    }
    
    Write-Host "Running..."
    $output = Invoke-Expression $Command 2>&1 | Out-String
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host $output
        
        # Parse output
        $msgPerSec = 0
        $cyclesPerMsg = 0
        
        if ($output -match 'Throughput:\s+(\d+(?:\.\d+)?)\s*M\s*msg/sec') {
            $msgPerSec = [double]$Matches[1] * 1000000
        }
        
        if ($output -match 'Cycles/msg:\s+(\d+(?:\.\d+)?)') {
            $cyclesPerMsg = [double]$Matches[1]
        }
        
        if ($msgPerSec -gt 0) {
            $script:results.benchmarks[$Name] = @{
                runtime = ""
                msg_per_sec = [int]$msgPerSec
                cycles_per_msg = $cyclesPerMsg
                notes = ""
            }
            Write-Host "OK $Name : $([int]($msgPerSec/1000000))M msg/sec" -ForegroundColor Green
        }
    } else {
        Write-Host "X Failed to run $Name" -ForegroundColor Red
    }
}

# Benchmark Aether (use existing)
$aetherExe = "..\..\tests\runtime\bench_batched_atomic.exe"
if (Test-Path $aetherExe) {
    Run-Benchmark -Name "Aether" -Command $aetherExe
    if ($script:results.benchmarks.ContainsKey("Aether")) {
        $script:results.benchmarks["Aether"].runtime = "Native C"
        $script:results.benchmarks["Aether"].notes = "Batched atomic updates"
    }
} else {
    Write-Host "`nX Aether benchmark not found at $aetherExe" -ForegroundColor Yellow
}

# Benchmark C++
$cppSrc = "cpp\ping_pong.cpp"
$cppExe = "cpp\ping_pong.exe"
if (Test-Path $cppSrc) {
    Run-Benchmark -Name "C++" `
        -BuildCommand "g++ -O3 -std=c++17 -march=native -o $cppExe $cppSrc -lpthread" `
        -Command $cppExe
    if ($script:results.benchmarks.ContainsKey("C++")) {
        $script:results.benchmarks["C++"].runtime = "Native"
        $script:results.benchmarks["C++"].notes = "std::thread + std::atomic"
    }
}

# Benchmark Go
$goSrc = "go\ping_pong.go"
if (Test-Path $goSrc) {
    # Check if Go is installed
    try {
        $goVersion = & go version 2>&1
        Run-Benchmark -Name "Go" `
            -BuildCommand "cd go; go build -o ping_pong.exe ping_pong.go; cd .." `
            -Command "go\ping_pong.exe"
        if ($script:results.benchmarks.ContainsKey("Go")) {
            $script:results.benchmarks["Go"].runtime = "Go runtime"
            $script:results.benchmarks["Go"].notes = "Goroutines + channels"
        }
    } catch {
        Write-Host "`nWarning: Go not installed, skipping Go benchmark" -ForegroundColor Yellow
    }
}

# Benchmark Rust
$rustSrc = "rust\Cargo.toml"
if (Test-Path $rustSrc) {
    # Check if Rust is installed
    try {
        $cargoVersion = & cargo --version 2>&1
        Run-Benchmark -Name "Rust" `
            -BuildCommand "cd rust; cargo build --release 2>&1 | Out-Null; cd .." `
            -Command "rust\target\release\ping_pong.exe"
        if ($script:results.benchmarks.ContainsKey("Rust")) {
            $script:results.benchmarks["Rust"].runtime = "Tokio async"
            $script:results.benchmarks["Rust"].notes = "Async channels"
        }
    } catch {
        Write-Host "`nWarning: Rust not installed, skipping Rust benchmark" -ForegroundColor Yellow
    }
}

# Save results
$jsonOutput = $results | ConvertTo-Json -Depth 10 -Compress:$false
$jsonOutput | Out-File -FilePath "visualize\results.json" -Encoding UTF8 -NoNewline

Write-Host "`n" -NoNewline
Write-Host "=" -NoNewline; Write-Host ("=" * 59)
Write-Host "  Summary"
Write-Host "=" -NoNewline; Write-Host ("=" * 59)

$sorted = $results.benchmarks.GetEnumerator() | Sort-Object { $_.Value.msg_per_sec } -Descending
foreach ($item in $sorted) {
    $name = $item.Key
    $msgPerSec = [int]($item.Value.msg_per_sec / 1000000)
    Write-Host ("{0,-15} {1,8}M msg/sec" -f $name, $msgPerSec)
}

Write-Host "`nOK Results saved to visualize\results.json" -ForegroundColor Green
Write-Host "`nTo view results:" -ForegroundColor Cyan
Write-Host "  cd visualize" -ForegroundColor White
Write-Host "  .\server.exe" -ForegroundColor White
Write-Host "  Open http://localhost:8080" -ForegroundColor Yellow
