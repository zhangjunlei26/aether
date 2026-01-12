# Comprehensive Cross-Language Benchmark Suite
# Tests: Ping-Pong, Ring, Skynet patterns
# Languages: Aether, C++, Go, Rust, Pony, Scala

$ErrorActionPreference = "Stop"

Write-Host "=" -NoNewline; Write-Host ("=" * 79)
Write-Host "  COMPREHENSIVE CROSS-LANGUAGE ACTOR BENCHMARK SUITE"
Write-Host "=" -NoNewline; Write-Host ("=" * 79)

# Hardware info
$cpu = (Get-WmiObject Win32_Processor).Name
$cores = (Get-WmiObject Win32_Processor).NumberOfLogicalProcessors
$os = "Windows"

Write-Host "CPU:   $cpu"
Write-Host "Cores: $cores"
Write-Host "OS:    $os"
Write-Host "=" -NoNewline; Write-Host ("=" * 79)

# Test patterns
$patterns = @("ping_pong", "ring", "skynet")
$languages = @("aether", "cpp", "go", "rust", "pony", "scala")

# Results storage
$allResults = @{}

foreach ($pattern in $patterns) {
    Write-Host "`n`n### PATTERN: $($pattern.ToUpper()) ###`n" -ForegroundColor Cyan
    
    $results = @{
        pattern = $pattern
        timestamp = (Get-Date).ToUniversalTime().ToString("yyyy-MM-ddTHH:mm:ssZ")
        hardware = @{
            cpu = $cpu
            cores = $cores
            os = $os
        }
        benchmarks = @{}
    }
    
    # Aether
    if ($pattern -eq "ping_pong") {
        $exe = "..\..\tests\runtime\bench_batched_atomic.exe"
    } else {
        $exe = "aether\$pattern.exe"
    }
    
    if (Test-Path $exe) {
        Write-Host "=== Aether ($pattern) ===" -ForegroundColor Green
        $output = & $exe 2>&1 | Out-String
        Write-Host $output
        
        if ($output -match 'Throughput:\s+(\d+)\s*M\s*msg/sec') {
            $msgPerSec = [int]$Matches[1] * 1000000
            $cycles = 0
            if ($output -match 'Cycles/msg:\s+([\d.]+)') {
                $cycles = [double]$Matches[1]
            }
            $results.benchmarks["Aether"] = @{
                runtime = "Native C"
                msg_per_sec = $msgPerSec
                cycles_per_msg = $cycles
                notes = "Batched atomic optimization"
            }
        }
    }
    
    # C++
    $cppExe = "cpp\$pattern.exe"
    if (Test-Path $cppExe) {
        Write-Host "`n=== C++ ($pattern) ===" -ForegroundColor Green
        $output = & $cppExe 2>&1 | Out-String
        Write-Host $output
        
        if ($output -match 'Throughput:\s+(\d+)\s*M\s*msg/sec') {
            $msgPerSec = [int]$Matches[1] * 1000000
            $cycles = 0
            if ($output -match 'Cycles/msg:\s+([\d.]+)') {
                $cycles = [double]$Matches[1]
            }
            $results.benchmarks["C++"] = @{
                runtime = "Native"
                msg_per_sec = $msgPerSec
                cycles_per_msg = $cycles
                notes = "std::thread + std::atomic"
            }
        }
    }
    
    # Go
    $goExe = "go\$pattern.exe"
    if (Test-Path $goExe) {
        Write-Host "`n=== Go ($pattern) ===" -ForegroundColor Green
        $output = & $goExe 2>&1 | Out-String
        Write-Host $output
        
        if ($output -match 'Throughput:\s+(\d+)\s*M\s*msg/sec') {
            $msgPerSec = [int]$Matches[1] * 1000000
            $cycles = 0
            if ($output -match 'Cycles/msg:\s+([\d.]+)') {
                $cycles = [double]$Matches[1]
            }
            $results.benchmarks["Go"] = @{
                runtime = "Go runtime"
                msg_per_sec = $msgPerSec
                cycles_per_msg = $cycles
                notes = "Goroutines + channels"
            }
        }
    }
    
    # Rust
    if ($pattern -eq "ping_pong") {
        $rustExe = "rust\target\release\ping_pong.exe"
    } else {
        $rustExe = "rust\target\release\$pattern.exe"
    }
    
    if (Test-Path $rustExe) {
        Write-Host "`n=== Rust ($pattern) ===" -ForegroundColor Green
        $output = & $rustExe 2>&1 | Out-String
        Write-Host $output
        
        if ($output -match 'Throughput:\s+(\d+)\s*M\s*msg/sec') {
            $msgPerSec = [int]$Matches[1] * 1000000
            $cycles = 0
            if ($output -match 'Cycles/msg:\s+([\d.]+)') {
                $cycles = [double]$Matches[1]
            }
            $results.benchmarks["Rust"] = @{
                runtime = "Tokio async"
                msg_per_sec = $msgPerSec
                cycles_per_msg = $cycles
                notes = "Async channels"
            }
        }
    }
    
    # Save pattern results
    $jsonOutput = $results | ConvertTo-Json -Depth 10 -Compress:$false
    $jsonOutput | Out-File -FilePath "visualize\results_$pattern.json" -Encoding UTF8 -NoNewline
    
    # Store in combined results
    $allResults[$pattern] = $results
}

# Create combined results
Write-Host "`n`n" -NoNewline
Write-Host "=" -NoNewline; Write-Host ("=" * 79)
Write-Host "  SUMMARY - ALL PATTERNS"
Write-Host "=" -NoNewline; Write-Host ("=" * 79)

foreach ($pattern in $patterns) {
    if ($allResults.ContainsKey($pattern)) {
        Write-Host "`n$($pattern.ToUpper()):" -ForegroundColor Cyan
        $sorted = $allResults[$pattern].benchmarks.GetEnumerator() | 
            Sort-Object { $_.Value.msg_per_sec } -Descending
        
        foreach ($item in $sorted) {
            $name = $item.Key
            $msgPerSec = [int]($item.Value.msg_per_sec / 1000000)
            Write-Host ("  {0,-15} {1,8}M msg/sec" -f $name, $msgPerSec)
        }
    }
}

# Use ping_pong as default for dashboard
if (Test-Path "visualize\results_ping_pong.json") {
    Copy-Item "visualize\results_ping_pong.json" "visualize\results.json" -Force
}

Write-Host "`n`nResults saved to visualize\results_*.json" -ForegroundColor Green
Write-Host "`nTo view dashboard:" -ForegroundColor Cyan
Write-Host "  cd visualize" -ForegroundColor White
Write-Host "  .\simple_server.exe" -ForegroundColor White
Write-Host "  Open http://localhost:8080" -ForegroundColor Yellow
