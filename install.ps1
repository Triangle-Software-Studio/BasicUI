#Requires -Version 5.1
<#
.SYNOPSIS
    BasicUI Framework One-Click Installer
.DESCRIPTION
    Automatically downloads dependencies (SDL2, GLEW, FreeType),
    configures CMake, builds the project, and copies required DLLs.
.PARAMETER Compiler
    Compiler to use: MSVC (default) or MinGW
.PARAMETER InstallDir
    Directory to store third-party dependencies
#>
param(
    [ValidateSet("MSVC", "MinGW")]
    [string]$Compiler = "MSVC",
    [string]$InstallDir = "$PSScriptRoot\third_party"
)

$ErrorActionPreference = "Stop"

# ---------------------------------------------------------------------------
# Dependency versions and URLs
# ---------------------------------------------------------------------------
$Deps = @{
    SDL2 = @{
        Version = "2.30.12"
        Url     = "https://github.com/libsdl-org/SDL/releases/download/release-2.30.12/SDL2-devel-2.30.12-VC.zip"
        Archive = "SDL2.zip"
        DirName = "SDL2"
    }
    GLEW = @{
        Version = "2.2.0"
        Url     = "https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0-win32.zip"
        Archive = "glew.zip"
        DirName = "glew"
    }
    FreeType = @{
        Version = "latest"
        Url     = "https://github.com/ubawurinna/freetype-windows-binaries/archive/refs/heads/master.zip"
        Archive = "freetype.zip"
        DirName = "freetype"
    }
}

# ---------------------------------------------------------------------------
# Helper functions
# ---------------------------------------------------------------------------
function Write-Header($text) {
    Write-Host "`n========================================" -ForegroundColor Cyan
    Write-Host "  $text" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
}

function Write-Step($text) {
    Write-Host "[+] $text" -ForegroundColor Green
}

function Write-Warn($text) {
    Write-Host "[!] $text" -ForegroundColor Yellow
}

function Write-ErrorAndExit($text) {
    Write-Host "[X] $text" -ForegroundColor Red
    exit 1
}

function Download-File($url, $dest) {
    $maxRetries = 3
    for ($i = 1; $i -le $maxRetries; $i++) {
        try {
            Write-Step "Downloading from $url (attempt $i/$maxRetries)..."
            Invoke-WebRequest -Uri $url -OutFile $dest -UseBasicParsing -TimeoutSec 120
            return
        } catch {
            Write-Warn "Download failed: $_"
            if ($i -eq $maxRetries) {
                Write-ErrorAndExit "Failed to download after $maxRetries attempts."
            }
            Start-Sleep -Seconds 5
        }
    }
}

function Ensure-Tool($name, $cmd) {
    try {
        $null = Get-Command $cmd -ErrorAction Stop
        Write-Step "$name found."
        return $true
    } catch {
        return $false
    }
}

# ---------------------------------------------------------------------------
# 0. Preflight checks
# ---------------------------------------------------------------------------
Write-Header "BasicUI Framework Installer"
Write-Host "Compiler: $Compiler"
Write-Host "Install directory: $InstallDir"

if (-not (Ensure-Tool "CMake" "cmake")) {
    Write-ErrorAndExit "CMake not found. Please install CMake 3.16+ from https://cmake.org/download/"
}

if ($Compiler -eq "MSVC") {
    if (-not (Ensure-Tool "MSBuild" "msbuild")) {
        Write-Warn "MSBuild not found in PATH."
        Write-Warn "Please install Visual Studio 2022 with 'Desktop development with C++' workload."
        Write-ErrorAndExit "MSVC toolchain missing."
    }
} else {
    if (-not (Ensure-Tool "MinGW Make" "mingw32-make")) {
        Write-ErrorAndExit "mingw32-make not found. Please install MinGW-w64 and add it to PATH."
    }
}

# ---------------------------------------------------------------------------
# 1. Create directories
# ---------------------------------------------------------------------------
Write-Header "Preparing directories"
New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null

# ---------------------------------------------------------------------------
# 2. Download dependencies
# ---------------------------------------------------------------------------
Write-Header "Downloading dependencies"

foreach ($key in $Deps.Keys) {
    $dep = $Deps[$key]
    $archivePath = Join-Path $InstallDir $dep.Archive
    $extractDir  = Join-Path $InstallDir $dep.DirName

    if (Test-Path $extractDir) {
        Write-Step "$key already present, skipping download."
        continue
    }

    if (-not (Test-Path $archivePath)) {
        Download-File $dep.Url $archivePath
    }

    Write-Step "Extracting $key..."
    Expand-Archive -Path $archivePath -DestinationPath $InstallDir -Force

    # Rename extracted folder to standardized name
    $extracted = Get-ChildItem $InstallDir -Directory | Where-Object {
        $_.Name -ne $dep.DirName -and $_.Name -notmatch "third_party"
    } | Sort-Object LastWriteTime -Descending | Select-Object -First 1

    if ($extracted -and -not (Test-Path $extractDir)) {
        Rename-Item $extracted.FullName $dep.DirName
    }

    Remove-Item $archivePath -ErrorAction SilentlyContinue
}

# ---------------------------------------------------------------------------
# 3. Post-process: fix up directory layouts for CMake
# ---------------------------------------------------------------------------
Write-Header "Post-processing dependencies"

# SDL2: the VC zip already has cmake/include/lib layout, just ensure cmake dir exists
$sdl2CMake = Join-Path $InstallDir "SDL2\cmake"
if (-not (Test-Path $sdl2CMake)) {
    # Some SDL2 packages name it differently; search for sdl2-config.cmake
    $sdl2Root = Join-Path $InstallDir "SDL2"
    $found = Get-ChildItem $sdl2Root -Recurse -Filter "sdl2-config.cmake" -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($found) {
        $cmakeDir = $found.DirectoryName
        if ($cmakeDir -ne $sdl2CMake) {
            New-Item -ItemType Junction -Path $sdl2CMake -Target $cmakeDir -ErrorAction SilentlyContinue | Out-Null
        }
    }
}

# GLEW: ensure lib/Release/x64 exists
$glewLib = Join-Path $InstallDir "glew\lib\Release\x64"
if (-not (Test-Path $glewLib)) {
    # Create expected structure
    New-Item -ItemType Directory -Force -Path $glewLib | Out-Null
    $srcLib = Join-Path $InstallDir "glew\lib"
    if (Test-Path $srcLib) {
        Get-ChildItem $srcLib -File | Copy-Item -Destination $glewLib -Force
    }
}

# FreeType (ubawurinna): rename win64 -> lib so CMake find_library works
$ftWin64 = Join-Path $InstallDir "freetype\win64"
$ftLib   = Join-Path $InstallDir "freetype\lib"
if ((Test-Path $ftWin64) -and -not (Test-Path $ftLib)) {
    Rename-Item $ftWin64 $ftLib
}
# Also ensure a Release subdir exists for our CMake fallback
$ftRelease = Join-Path $InstallDir "freetype\build\Release"
if (-not (Test-Path $ftRelease)) {
    New-Item -ItemType Directory -Force -Path $ftRelease | Out-Null
    Get-ChildItem $ftLib -File | Copy-Item -Destination $ftRelease -Force
}

# ---------------------------------------------------------------------------
# 4. Configure with CMake
# ---------------------------------------------------------------------------
Write-Header "Configuring with CMake"

$projectRoot = $PSScriptRoot
$buildDir    = Join-Path $projectRoot "build"

$sdl2Dir     = Join-Path $InstallDir "SDL2\cmake"
$glewDir     = Join-Path $InstallDir "glew"
$freetypeDir = Join-Path $InstallDir "freetype"

$cmakeArgs = @(
    "-B", "$buildDir"
    "-S", "$projectRoot"
    "-DSDL2_DIR=$sdl2Dir"
    "-DGLEW_DIR=$glewDir"
    "-DFREETYPE_DIR=$freetypeDir"
)

if ($Compiler -eq "MSVC") {
    $cmakeArgs += "-G", "Visual Studio 17 2022", "-A", "x64"
} else {
    $cmakeArgs += "-G", "MinGW Makefiles", "-DCMAKE_BUILD_TYPE=Release"
}

Write-Step "Running: cmake $cmakeArgs"
& cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) {
    Write-ErrorAndExit "CMake configuration failed."
}

# ---------------------------------------------------------------------------
# 5. Build
# ---------------------------------------------------------------------------
Write-Header "Building BasicUI"

if ($Compiler -eq "MSVC") {
    & cmake --build $buildDir --config Release --target basicui_demo
} else {
    & cmake --build $buildDir --target basicui_demo
}

if ($LASTEXITCODE -ne 0) {
    Write-ErrorAndExit "Build failed."
}

# ---------------------------------------------------------------------------
# 6. Copy DLLs to output directory
# ---------------------------------------------------------------------------
Write-Header "Copying runtime DLLs"

$outDir = if ($Compiler -eq "MSVC") { Join-Path $buildDir "Release" } else { Join-Path $buildDir "" }
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

$dllMap = @{
    "SDL2"     = @("SDL2\lib\x64\SDL2.dll", "SDL2\lib\x86\SDL2.dll")
    "GLEW"     = @("glew\bin\Release\x64\glew32.dll")
    "FreeType" = @("freetype\lib\freetype.dll", "freetype\build\Release\freetype.dll")
}

foreach ($key in $dllMap.Keys) {
    $candidates = $dllMap[$key]
    $copied = $false
    foreach ($relPath in $candidates) {
        $src = Join-Path $InstallDir $relPath
        if (Test-Path $src) {
            Copy-Item $src $outDir -Force
            Write-Step "Copied $key DLL: $relPath"
            $copied = $true
            break
        }
    }
    if (-not $copied) {
        Write-Warn "Could not find $key DLL. You may need to manually copy it to the output folder."
    }
}

# Also copy fonts if they exist
$fontSrc = Join-Path $projectRoot "fonts"
$fontDst = Join-Path $outDir "fonts"
if (Test-Path $fontSrc) {
    if (-not (Test-Path $fontDst)) {
        New-Item -ItemType Junction -Path $fontDst -Target $fontSrc -ErrorAction SilentlyContinue | Out-Null
    }
    Write-Step "Linked fonts directory."
}

# ---------------------------------------------------------------------------
# 7. Done
# ---------------------------------------------------------------------------
Write-Header "Installation Complete"
Write-Host "Executable: $outDir\basicui_demo.exe" -ForegroundColor Green
Write-Host "`nNext steps:" -ForegroundColor White
Write-Host "  1. Ensure SDL2.dll, glew32.dll, freetype.dll are next to the exe (already copied)."
Write-Host "  2. Run: $outDir\basicui_demo.exe"
Write-Host "  3. To use the framework in your own project, include 'include/' and link against 'build/Release/basicui.lib'."
