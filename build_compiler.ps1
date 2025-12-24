# Build Aether Compiler (Windows PowerShell)
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Building Aether Compiler" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

# Create build directory
if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
}

# Find compiler sources
$compilerSources = @(
    "compiler\aetherc.c",
    "compiler\lexer.c",
    "compiler\parser.c",
    "compiler\ast.c",
    "compiler\typechecker.c",
    "compiler\codegen.c",
    "compiler\aether_error.c",
    "compiler\aether_module.c",
    "compiler\type_inference.c"
)

# Check if gcc is available
$gccPath = Get-Command gcc -ErrorAction SilentlyContinue

if (-not $gccPath) {
    Write-Host "ERROR: gcc not found in PATH" -ForegroundColor Red
    Write-Host "Please ensure MinGW, MSYS2, or similar is installed and in your PATH" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Quick install options:" -ForegroundColor Yellow
    Write-Host "  1. MSYS2: https://www.msys2.org/" -ForegroundColor Gray
    Write-Host "  2. MinGW-w64: https://www.mingw-w64.org/" -ForegroundColor Gray
    Write-Host "  3. WSL (Ubuntu): wsl --install" -ForegroundColor Gray
    exit 1
}

Write-Host "Using: $($gccPath.Source)" -ForegroundColor Gray
Write-Host ""

# Build command
$buildCmd = "gcc " + ($compilerSources -join " ") + " -o build\aetherc.exe -Icompiler -Iruntime -Wall -O2"

Write-Host "Compiling..." -ForegroundColor Yellow
Invoke-Expression $buildCmd

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "Build successful!" -ForegroundColor Green
    Write-Host "Compiler: build\aetherc.exe" -ForegroundColor Cyan
} else {
    Write-Host ""
    Write-Host "Build failed with exit code $LASTEXITCODE" -ForegroundColor Red
    exit $LASTEXITCODE
}

