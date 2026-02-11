# ==============================================================================
# Filament Engine — Setup (Windows)
#
# Downloads third-party dependencies (EnTT, Filament pre-built, SDL2)
# and optionally configures CMake.
#
# Usage:
#   .\scripts\setup.ps1
#   .\scripts\setup.ps1 -NoCmake
#   .\scripts\setup.ps1 -DepsOnly
#
# ==============================================================================
param(
    [switch]$NoCmake,
    [switch]$DepsOnly,
    [switch]$Help
)

$ErrorActionPreference = "Stop"

if ($Help) {
    Write-Host @"
Usage: .\scripts\setup.ps1 [OPTIONS]

Options:
  -NoCmake    Download dependencies only (skip CMake configure)
  -DepsOnly   Same as -NoCmake
  -Help       Show this help
"@
    exit 0
}

if ($DepsOnly) { $NoCmake = $true }

# -------------------------
# Paths & versions
# -------------------------
$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$VendorDir   = Join-Path $ProjectRoot "vendor"
$BuildDir    = Join-Path $ProjectRoot "build"

$FilamentVersion = "v1.69.2"
$EnttVersion     = "v3.16.0"
$Sdl2Version     = "2.30.0"

# -------------------------
# Logging helpers
# -------------------------
function Info($msg)    { Write-Host "[INFO] $msg" -ForegroundColor Green }
function Warn($msg)    { Write-Host "[WARN] $msg" -ForegroundColor Yellow }
function Err($msg)     { Write-Host "[ERROR] $msg" -ForegroundColor Red }
function Section($msg) { Write-Host "`n=== $msg ===" -ForegroundColor Cyan }

function Die($msg) {
    Err $msg
    exit 1
}

# -------------------------
# Prerequisite checks
# -------------------------
function Test-Command($cmd) {
    return [bool](Get-Command $cmd -ErrorAction SilentlyContinue)
}

function Ensure-Prerequisites {
    Section "System prerequisites"

    $missing = @()
    if (-not (Test-Command "git"))   { $missing += "git" }
    if (-not (Test-Command "cmake")) { $missing += "cmake" }
    if (-not (Test-Command "curl"))  { $missing += "curl" }

    if ($missing.Count -gt 0) {
        Die "Missing required tools: $($missing -join ', '). Please install them and try again."
    }

    # Ninja is optional but recommended
    if (-not (Test-Command "ninja")) {
        Warn "Ninja not found. CMake will use the Visual Studio generator (slower)."
        Warn "Install Ninja for faster builds: winget install Ninja-build.Ninja"
    }

    Info "Required tools found."
}

# ==============================================================================
Write-Host "==========================================="
Write-Host " Filament Engine - Setup (Windows)"
Write-Host "==========================================="
Info "Project root: $ProjectRoot"

Ensure-Prerequisites

# ==============================================================================
# 1. Create vendor directory
# ==============================================================================
if (-not (Test-Path $VendorDir)) {
    New-Item -ItemType Directory -Path $VendorDir | Out-Null
}

# ==============================================================================
# 2. EnTT (header-only)
# ==============================================================================
Section "EnTT $EnttVersion"
$EnttDir = Join-Path $VendorDir "entt"

if (Test-Path (Join-Path $EnttDir "single_include\entt")) {
    Info "EnTT already present, skipping."
} else {
    Info "Cloning EnTT $EnttVersion..."
    if (Test-Path $EnttDir) { Remove-Item -Recurse -Force $EnttDir }
    & git clone --depth 1 --branch $EnttVersion "https://github.com/skypjack/entt.git" $EnttDir
    if ($LASTEXITCODE -ne 0) { Die "Failed to clone EnTT." }

    if (-not (Test-Path (Join-Path $EnttDir "single_include\entt\entt.hpp"))) {
        Die "EnTT clone succeeded but entt.hpp not found."
    }
    Info "EnTT $EnttVersion OK."
}

# ==============================================================================
# 3. Google Filament (pre-built)
# ==============================================================================
Section "Filament $FilamentVersion"
$FilamentArchive = Join-Path $VendorDir "filament.tgz"
$FilamentDir     = Join-Path $VendorDir "filament"

if ((Test-Path (Join-Path $FilamentDir "include")) -and
    (Test-Path (Join-Path $FilamentDir "lib")) -and
    (Test-Path (Join-Path $FilamentDir "bin"))) {
    Info "Filament distribution already present, skipping."
} else {
    # Download if archive not cached
    if (-not (Test-Path $FilamentArchive)) {
        $Url = "https://github.com/google/filament/releases/download/$FilamentVersion/filament-$FilamentVersion-windows.tgz"
        Info "Downloading Filament $FilamentVersion..."
        Info "URL: $Url"
        & curl.exe -fSL --retry 3 --retry-delay 2 --progress-bar -o $FilamentArchive $Url
        if ($LASTEXITCODE -ne 0) {
            if (Test-Path $FilamentArchive) { Remove-Item $FilamentArchive }
            Die "Failed to download Filament."
        }
    } else {
        Info "Found cached Filament archive."
    }

    # Extract
    Info "Extracting Filament..."
    if (Test-Path $FilamentDir) { Remove-Item -Recurse -Force $FilamentDir }
    New-Item -ItemType Directory -Path $FilamentDir -Force | Out-Null
    & tar -xzf $FilamentArchive -C $FilamentDir --strip-components=0

    if ($LASTEXITCODE -ne 0) { Die "Failed to extract Filament archive." }

    # Validate
    if (-not (Test-Path (Join-Path $FilamentDir "include")) -or
        -not (Test-Path (Join-Path $FilamentDir "lib")) -or
        -not (Test-Path (Join-Path $FilamentDir "bin"))) {
        Die "Filament extraction incomplete: missing include/, lib/, or bin/ directory."
    }
    Info "Filament $FilamentVersion OK."
}

# ==============================================================================
# 4. SDL2
# ==============================================================================
Section "SDL2 $Sdl2Version"
$Sdl2Archive = Join-Path $VendorDir "SDL2.zip"
$Sdl2Dir     = Join-Path $VendorDir "SDL2"

if ((Test-Path (Join-Path $Sdl2Dir "include")) -and
    (Test-Path (Join-Path $Sdl2Dir "lib\x64\SDL2.dll"))) {
    Info "SDL2 already present, skipping."
} else {
    if (-not (Test-Path $Sdl2Archive)) {
        $Url = "https://github.com/libsdl-org/SDL/releases/download/release-$Sdl2Version/SDL2-devel-$Sdl2Version-VC.zip"
        Info "Downloading SDL2 $Sdl2Version..."
        & curl.exe -fSL --retry 3 --retry-delay 2 --progress-bar -o $Sdl2Archive $Url
        if ($LASTEXITCODE -ne 0) {
            if (Test-Path $Sdl2Archive) { Remove-Item $Sdl2Archive }
            Die "Failed to download SDL2."
        }
    } else {
        Info "Found cached SDL2 archive."
    }

    Info "Extracting SDL2..."
    if (Test-Path $Sdl2Dir) { Remove-Item -Recurse -Force $Sdl2Dir }
    Expand-Archive -Path $Sdl2Archive -DestinationPath $VendorDir -Force

    # The zip extracts to SDL2-<version>, rename
    $Extracted = Join-Path $VendorDir "SDL2-$Sdl2Version"
    if (Test-Path $Extracted) {
        Move-Item $Extracted $Sdl2Dir
    }

    # Validate
    if (-not (Test-Path (Join-Path $Sdl2Dir "include\SDL.h"))) {
        Die "SDL2 extraction incomplete: SDL.h not found."
    }
    Info "SDL2 $Sdl2Version OK."
}

# ==============================================================================
# 5. Configure CMake
# ==============================================================================
if ($NoCmake) {
    Warn "Skipping CMake configuration (-NoCmake)."
} else {
    Section "CMake Configuration"

    if (-not (Test-Path $BuildDir)) {
        New-Item -ItemType Directory -Path $BuildDir | Out-Null
    }

    $cmakeArgs = @(
        "-S", $ProjectRoot,
        "-B", $BuildDir,
        "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    )

    # Prefer Ninja if available
    if (Test-Command "ninja") {
        Info "Using Ninja generator."
        $cmakeArgs += @("-G", "Ninja")
    } else {
        Info "Using default generator (Visual Studio)."
    }

    & cmake @cmakeArgs
    if ($LASTEXITCODE -ne 0) { Die "CMake configuration failed." }
    Info "CMake configured."
}

# ==============================================================================
# Summary
# ==============================================================================
Write-Host ""
Write-Host "==========================================="
Info "Setup complete!"
Write-Host ""
Write-Host "  Dependencies:"
Write-Host "    - EnTT $EnttVersion"
Write-Host "    - Filament $FilamentVersion (pre-built)"
Write-Host "    - SDL2 $Sdl2Version"
Write-Host ""
Write-Host "  Next steps:"
Write-Host "    .\scripts\build.ps1            # Build the engine"
Write-Host "    .\scripts\build.ps1 --clean    # Clean rebuild"
Write-Host "==========================================="
