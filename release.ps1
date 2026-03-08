# KType release script
# Builds Release, creates a GitHub release, and uploads the binary.
#
# Usage:
#   .\release.ps1                    # Auto-increment patch version (v0.1.0 → v0.1.1)
#   .\release.ps1 -Version v0.2.0   # Specific version
#   .\release.ps1 -Draft             # Create as draft release
#   .\release.ps1 -Overwrite         # Overwrite existing release (for dev iterations)

param(
    [string]$Version,
    [switch]$Draft,
    [switch]$Overwrite
)

$ErrorActionPreference = "Stop"

# Ensure gh CLI is available
if (-not (Get-Command gh -ErrorAction SilentlyContinue)) {
    Write-Error "GitHub CLI (gh) is required. Install: winget install GitHub.cli"
    exit 1
}

# Auto-detect version from latest release if not specified
if (-not $Version) {
    $latest = gh release list --limit 1 --json tagName --jq '.[0].tagName' 2>$null
    if ($latest -match '^v(\d+)\.(\d+)\.(\d+)$') {
        $major = [int]$matches[1]
        $minor = [int]$matches[2]
        $patch = [int]$matches[3] + 1
        $Version = "v$major.$minor.$patch"
    } else {
        $Version = "v0.1.0"
    }
    Write-Host "Version: $Version (auto-detected)" -ForegroundColor Cyan
}

# Validate version format
if ($Version -notmatch '^v\d+\.\d+\.\d+$') {
    Write-Error "Invalid version format: $Version (expected vX.Y.Z)"
    exit 1
}

Write-Host "Building Release..." -ForegroundColor Yellow

# Build
$msbuild = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
if (-not (Test-Path $msbuild)) {
    Write-Error "MSBuild not found at $msbuild"
    exit 1
}

& $msbuild KType.sln /p:Configuration=Release /p:Platform=x64 /nologo /v:minimal
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed"
    exit 1
}

# Run tests
Write-Host "Running tests..." -ForegroundColor Yellow
$testExe = "tests\build\x64\Release\tests.exe"
if (-not (Test-Path $testExe)) {
    # Tests might only build in Debug
    & $msbuild tests\tests.vcxproj /p:Configuration=Debug /p:Platform=x64 /nologo /v:minimal
    $testExe = "tests\build\x64\Debug\tests.exe"
}
& $testExe
if ($LASTEXITCODE -ne 0) {
    Write-Error "Tests failed — aborting release"
    exit 1
}

# Package
$releaseDir = "build\x64\Release"
$zipName = "KType-$Version-x64.zip"
$zipPath = "build\$zipName"

Write-Host "Packaging $zipName..." -ForegroundColor Yellow

# Collect release files
$stagingDir = "build\_release_staging"
if (Test-Path $stagingDir) { Remove-Item $stagingDir -Recurse -Force }
New-Item $stagingDir -ItemType Directory | Out-Null

# Copy DLL and related files
$filesToCopy = @("KType.dll", "KType.pdb")
foreach ($f in $filesToCopy) {
    $src = Join-Path $releaseDir $f
    if (Test-Path $src) {
        Copy-Item $src $stagingDir
    }
}

# Copy docs
Copy-Item "README.md" $stagingDir -ErrorAction SilentlyContinue
Copy-Item "LICENSE" $stagingDir -ErrorAction SilentlyContinue

# Create zip
if (Test-Path $zipPath) { Remove-Item $zipPath -Force }
Compress-Archive -Path "$stagingDir\*" -DestinationPath $zipPath
Remove-Item $stagingDir -Recurse -Force

if (-not (Test-Path $zipPath)) {
    Write-Error "Failed to create $zipPath"
    exit 1
}

Write-Host "Created $zipPath" -ForegroundColor Green

# Create GitHub release
Write-Host "Creating GitHub release $Version..." -ForegroundColor Yellow

$ghArgs = @("release", "create", $Version, $zipPath, "--title", "KType $Version")

if ($Draft) {
    $ghArgs += "--draft"
}

if ($Overwrite) {
    # Delete existing release first
    gh release delete $Version --yes 2>$null
    git tag -d $Version 2>$null
    git push origin --delete $Version 2>$null
}

$ghArgs += "--notes"
$ghArgs += "KType $Version - Vietnamese Telex IME for Windows (TSF)`n`nSee README.md for installation instructions."

gh @ghArgs
if ($LASTEXITCODE -ne 0) {
    Write-Error "Failed to create GitHub release"
    exit 1
}

Write-Host "`nRelease $Version published!" -ForegroundColor Green
Write-Host "Download: gh release download $Version" -ForegroundColor Cyan
