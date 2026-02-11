# ==============================================================================
# Filament Engine — Build (Windows)
#
# Configures (if needed) and compiles the project using Ninja (preferred)
# or the Visual Studio generator.
#
# Usage:
#   .\scripts\build.ps1
#   .\scripts\build.ps1 -Clean
#   .\scripts\build.ps1 -Reconfigure
#   .\scripts\build.ps1 -BuildType Debug
#
# ==============================================================================
param(
    [switch]$Clean,
    [switch]$Reconfigure,
    [ValidateSet("Release", "Debug")]
    [string]$BuildType = "Release",
    [switch]$Help
)

$ErrorActionPreference = "Stop"

if ($Help) {
    Write-Host @"
Usage: .\scripts\build.ps1 [OPTIONS]

Options:
  -Clean          Remove build directory before configuring/building
  -Reconfigure    Force CMake reconfigure even if already configured
  -BuildType      Release or Debug (default: Release)
  -Help           Show this help
"@
    exit 0
}

# -------------------------
# Paths
# -------------------------
$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$BuildDir    = Join-Path $ProjectRoot "build"
$ScriptsDir  = Join-Path $ProjectRoot "scripts"

# -------------------------
# Logging helpers
# -------------------------
function Info($msg)    { Write-Host "[INFO] $msg" -ForegroundColor Green }
function Warn($msg)    { Write-Host "[WARN] $msg" -ForegroundColor Yellow }
function Err($msg)     { Write-Host "[ERROR] $msg" -ForegroundColor Red }
function Die($msg)     { Err $msg; exit 1 }

function Test-Command($cmd) {
    return [bool](Get-Command $cmd -ErrorAction SilentlyContinue)
}

# -------------------------
# Tool checks
# -------------------------
if (-not (Test-Command "cmake")) { Die "cmake not found. Please install CMake." }

$UseNinja = Test-Command "ninja"

# -------------------------
# CPU count
# -------------------------
$Jobs = (Get-CimInstance Win32_Processor | Measure-Object -Property NumberOfLogicalProcessors -Sum).Sum
if (-not $Jobs -or $Jobs -lt 1) { $Jobs = 4 }

# -------------------------
# Validate dependencies
# -------------------------
$FilamentDir = Join-Path $ProjectRoot "vendor\filament"
if (-not (Test-Path (Join-Path $FilamentDir "include"))) {
    Warn "Filament not found. Running setup..."
    & "$ScriptsDir\setup.ps1"
    if ($LASTEXITCODE -ne 0) { Die "Setup failed." }
}

# -------------------------
# Clean
# -------------------------
if ($Clean -and (Test-Path $BuildDir)) {
    Warn "Cleaning build directory..."
    Remove-Item -Recurse -Force $BuildDir
}

# -------------------------
# Check if already configured
# -------------------------
function Test-Configured {
    if ($UseNinja) {
        return (Test-Path (Join-Path $BuildDir "build.ninja")) -and
               (Test-Path (Join-Path $BuildDir "CMakeCache.txt"))
    } else {
        return (Test-Path (Join-Path $BuildDir "CMakeCache.txt"))
    }
}

# -------------------------
# Configure
# -------------------------
function Invoke-Configure {
    Info "Configuring CMake ($BuildType)..."

    if (-not (Test-Path $BuildDir)) {
        New-Item -ItemType Directory -Path $BuildDir | Out-Null
    }

    $cmakeArgs = @(
        "-S", $ProjectRoot,
        "-B", $BuildDir,
        "-DCMAKE_BUILD_TYPE=$BuildType",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    )

    if ($UseNinja) {
        $cmakeArgs += @("-G", "Ninja")
    }

    & cmake @cmakeArgs
    if ($LASTEXITCODE -ne 0) { Die "CMake configuration failed." }
    Info "CMake configured."
}

# -------------------------
# Ensure configured
# -------------------------
if (-not (Test-Configured)) {
    Warn "Build not configured. Configuring..."
    Invoke-Configure
} elseif ($Reconfigure) {
    Warn "Forcing reconfigure..."
    Invoke-Configure
} else {
    Info "Build already configured."
}

# -------------------------
# Build
# -------------------------
Write-Host "`n=== Filament Engine - Build ($BuildType, $Jobs jobs) ===" -ForegroundColor Cyan

if ($UseNinja) {
    & ninja -C $BuildDir -j $Jobs
} else {
    & cmake --build $BuildDir --config $BuildType --parallel $Jobs
}

if ($LASTEXITCODE -ne 0) { Die "Build failed." }

Write-Host ""
Info "Build complete."

# Show binary location
$SandboxExe = Join-Path $BuildDir "sandbox\sandbox.exe"
if (Test-Path $SandboxExe) {
    Info "Binary at: $SandboxExe"
}
