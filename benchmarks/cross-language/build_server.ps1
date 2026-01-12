# Build and run the Aether benchmark visualization server

Write-Host "=" -NoNewline; Write-Host ("=" * 59)
Write-Host "  Building Aether Benchmark Visualization Server"
Write-Host "=" -NoNewline; Write-Host ("=" * 59)

$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot\visualize

# Build the server
Write-Host "`nCompiling server.c..."
gcc -O2 -o server.exe server.c `
    -I../../std/net `
    -I../../runtime `
    ../../std/net/*.c `
    ../../runtime/*.c `
    ../../runtime/actors/*.c `
    ../../runtime/utils/*.c `
    ../../runtime/scheduler/*.c `
    -lws2_32 -lpthread 2>&1 | Out-String | Write-Host

if ($LASTEXITCODE -eq 0) {
    Write-Host "✓ Server built successfully!" -ForegroundColor Green
    Write-Host "`nStarting server on http://localhost:8080"
    Write-Host "Press Ctrl+C to stop"
    Write-Host ""
    
    # Run the server
    .\server.exe
} else {
    Write-Host "✗ Build failed!" -ForegroundColor Red
    exit 1
}
