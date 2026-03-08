# KType release script
# Builds Release, creates a GitHub release, and uploads the binary.
#
# Usage:
#   .\release.ps1                    # Auto-increment patch version (v0.1.0 -> v0.1.1)
#   .\release.ps1 -Version v0.2.0   # Specific version
#   .\release.ps1 -Draft             # Create as draft release
#   .\release.ps1 -Overwrite         # Overwrite existing release (for dev iterations)

param(
    [string]$Version,
    [switch]$Draft,
    [switch]$Overwrite
)

$ErrorActionPreference = "Stop"
$repo = "kienvtv3/ktype"

# --- Get GitHub token from git credential store ---
function Get-GitHubToken {
    $output = "protocol=https`nhost=github.com" | git credential fill 2>$null
    foreach ($line in $output -split "`n") {
        if ($line -match '^password=(.+)$') {
            return $matches[1]
        }
    }
    Write-Error "No GitHub token found in git credentials. Run: git credential fill"
    exit 1
}

# --- GitHub API helpers ---
function Invoke-GitHubApi {
    param([string]$Method, [string]$Uri, $Body, [string]$ContentType = 'application/json')
    $token = Get-GitHubToken
    $headers = @{
        'Authorization' = "token $token"
        'Accept' = 'application/vnd.github+json'
    }
    $params = @{
        Uri = $Uri
        Method = $Method
        Headers = $headers
        ContentType = $ContentType
    }
    if ($Body) { $params.Body = $Body }
    Invoke-RestMethod @params
}

# --- Auto-detect version from latest release ---
if (-not $Version) {
    try {
        $releases = Invoke-GitHubApi -Method Get -Uri "https://api.github.com/repos/$repo/releases?per_page=1"
        if ($releases -and $releases[0].tag_name -match '^v(\d+)\.(\d+)\.(\d+)$') {
            $major = [int]$matches[1]
            $minor = [int]$matches[2]
            $patch = [int]$matches[3] + 1
            $Version = "v$major.$minor.$patch"
        }
    } catch {}
    if (-not $Version) { $Version = "v0.1.0" }
    Write-Host "Version: $Version (auto-detected)" -ForegroundColor Cyan
}

if ($Version -notmatch '^v\d+\.\d+\.\d+$') {
    Write-Error "Invalid version format: $Version (expected vX.Y.Z)"
    exit 1
}

# --- Build ---
Write-Host "Building Release..." -ForegroundColor Yellow

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

# --- Run tests ---
Write-Host "Running tests..." -ForegroundColor Yellow
$testExe = "build\x64\Release\tests.exe"
if (-not (Test-Path $testExe)) {
    $testExe = "tests\build\x64\Release\tests.exe"
}
if (-not (Test-Path $testExe)) {
    & $msbuild tests\tests.vcxproj /p:Configuration=Debug /p:Platform=x64 /nologo /v:minimal
    $testExe = "tests\build\x64\Debug\tests.exe"
}
& $testExe
if ($LASTEXITCODE -ne 0) {
    Write-Error "Tests failed - aborting release"
    exit 1
}

# --- Package ---
$releaseDir = "build\x64\Release"
$zipName = "KType-$Version-x64.zip"
$zipPath = "build\$zipName"

Write-Host "Packaging $zipName..." -ForegroundColor Yellow

$stagingDir = "build\_release_staging"
if (Test-Path $stagingDir) { Remove-Item $stagingDir -Recurse -Force }
New-Item $stagingDir -ItemType Directory | Out-Null

foreach ($f in @("KType.dll", "KType.pdb")) {
    $src = Join-Path $releaseDir $f
    if (Test-Path $src) { Copy-Item $src $stagingDir }
}
Copy-Item "README.md" $stagingDir -ErrorAction SilentlyContinue
Copy-Item "LICENSE" $stagingDir -ErrorAction SilentlyContinue

if (Test-Path $zipPath) { Remove-Item $zipPath -Force }
Compress-Archive -Path "$stagingDir\*" -DestinationPath $zipPath
Remove-Item $stagingDir -Recurse -Force

if (-not (Test-Path $zipPath)) {
    Write-Error "Failed to create $zipPath"
    exit 1
}
Write-Host "Created $zipPath" -ForegroundColor Green

# --- Delete existing release if -Overwrite ---
if ($Overwrite) {
    Write-Host "Deleting existing release $Version..." -ForegroundColor Yellow
    try {
        $existing = Invoke-GitHubApi -Method Get -Uri "https://api.github.com/repos/$repo/releases/tags/$Version"
        Invoke-GitHubApi -Method Delete -Uri "https://api.github.com/repos/$repo/releases/$($existing.id)" | Out-Null
    } catch {}
    git tag -d $Version 2>$null
    git push origin --delete $Version 2>$null
}

# --- Create GitHub release ---
Write-Host "Creating GitHub release $Version..." -ForegroundColor Yellow

$releaseBody = @{
    tag_name = $Version
    name = "KType $Version"
    body = "KType $Version - Vietnamese Telex IME for Windows (TSF)`n`nSee README.md for installation instructions."
    draft = [bool]$Draft
    prerelease = $false
} | ConvertTo-Json -Compress

$release = Invoke-GitHubApi -Method Post -Uri "https://api.github.com/repos/$repo/releases" -Body $releaseBody
Write-Host "Release created: $($release.html_url)" -ForegroundColor Green

# --- Upload asset ---
Write-Host "Uploading $zipName..." -ForegroundColor Yellow

$uploadUrl = "https://uploads.github.com/repos/$repo/releases/$($release.id)/assets?name=$zipName"
$fileBytes = [System.IO.File]::ReadAllBytes((Resolve-Path $zipPath))
$token = Get-GitHubToken
$headers = @{
    'Authorization' = "token $token"
    'Accept' = 'application/vnd.github+json'
    'Content-Type' = 'application/zip'
}
$asset = Invoke-RestMethod -Uri $uploadUrl -Method Post -Headers $headers -Body $fileBytes
Write-Host "Uploaded: $($asset.browser_download_url)" -ForegroundColor Green

Write-Host "`nRelease $Version published!" -ForegroundColor Green
