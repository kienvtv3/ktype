# KType release script
# Builds Release, creates installer, publishes to GitHub.
#
# Usage:
#   .\release.ps1                    # Auto-increment patch version
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

# --- Paths ---
$msbuild = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
$iscc = "$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe"
$signtool = (Get-ChildItem "C:\Program Files (x86)\Windows Kits\10\bin\*\x64\signtool.exe" -ErrorAction SilentlyContinue | Sort-Object FullName -Descending | Select-Object -First 1).FullName

foreach ($tool in @(@($msbuild, "MSBuild"), @($iscc, "Inno Setup (ISCC)"))) {
    if (-not (Test-Path $tool[0])) {
        Write-Error "$($tool[1]) not found at $($tool[0])"
        exit 1
    }
}

# --- Find code signing certificate ---
$certName = "Kien Vu (KType)"
$signingCert = Get-ChildItem Cert:\CurrentUser\My -ErrorAction SilentlyContinue | Where-Object { $_.Subject -eq "CN=$certName" -and $_.EnhancedKeyUsageList.ObjectId -contains "1.3.6.1.5.5.7.3.3" }
$canSign = $signingCert -and $signtool
if (-not $canSign) {
    Write-Host "WARNING: Code signing unavailable." -ForegroundColor DarkYellow
    if (-not $signingCert) { Write-Host "  No certificate. Run: .\create-cert.ps1 (as admin)" -ForegroundColor DarkYellow }
    if (-not $signtool) { Write-Host "  signtool.exe not found. Install Windows SDK." -ForegroundColor DarkYellow }
}

# --- Get GitHub token from git credential store ---
function Get-GitHubToken {
    $prev = $ErrorActionPreference
    $ErrorActionPreference = "SilentlyContinue"
    $output = "protocol=https`nhost=github.com" | git credential fill 2>$null
    $ErrorActionPreference = $prev
    foreach ($line in $output -split "`n") {
        if ($line -match '^password=(.+)$') { return $matches[1] }
    }
    Write-Error "No GitHub token found. Run: git credential fill"
    exit 1
}

function Invoke-GitHubApi {
    param([string]$Method, [string]$Uri, $Body, [string]$ContentType = 'application/json')
    $headers = @{
        'Authorization' = "token $(Get-GitHubToken)"
        'Accept' = 'application/vnd.github+json'
    }
    $params = @{ Uri = $Uri; Method = $Method; Headers = $headers; ContentType = $ContentType }
    if ($Body) { $params.Body = $Body }
    Invoke-RestMethod @params
}

# --- Auto-detect version ---
if (-not $Version) {
    try {
        $releases = Invoke-GitHubApi -Method Get -Uri "https://api.github.com/repos/$repo/releases?per_page=1"
        if ($releases -and $releases[0].tag_name -match '^v(\d+)\.(\d+)\.(\d+)$') {
            $Version = "v$([int]$matches[1]).$([int]$matches[2]).$([int]$matches[3] + 1)"
        }
    } catch {}
    if (-not $Version) { $Version = "v0.1.0" }
    Write-Host "Version: $Version (auto)" -ForegroundColor Cyan
}

$versionNum = $Version -replace '^v', ''
if ($versionNum -notmatch '^\d+\.\d+\.\d+$') {
    Write-Error "Invalid version: $Version (expected vX.Y.Z)"
    exit 1
}

# --- Build Release ---
Write-Host "`n[1/4] Building Release..." -ForegroundColor Yellow
& $msbuild KType.sln /p:Configuration=Release /p:Platform=x64 /nologo /v:minimal
if ($LASTEXITCODE -ne 0) { Write-Error "Build failed"; exit 1 }

# --- Sign DLL ---
if ($canSign) {
    Write-Host "`nSigning KType.dll..." -ForegroundColor Yellow
    & $signtool sign /a /s My /n $certName /fd sha256 /td sha256 /tr http://timestamp.digicert.com "build\x64\Release\KType.dll"
    if ($LASTEXITCODE -ne 0) { Write-Host "DLL signing failed (continuing)" -ForegroundColor DarkYellow }
    else { Write-Host "DLL signed" -ForegroundColor Green }
}

# --- Run tests ---
Write-Host "`n[2/4] Running tests..." -ForegroundColor Yellow
$testExe = "build\x64\Release\tests.exe"
if (-not (Test-Path $testExe)) { $testExe = "tests\build\x64\Release\tests.exe" }
& $testExe
if ($LASTEXITCODE -ne 0) { Write-Error "Tests failed - aborting"; exit 1 }

# --- Build installer ---
Write-Host "`n[3/4] Building installer..." -ForegroundColor Yellow
& $iscc /DMyAppVersion=$versionNum installer\ktype.iss /Q
if ($LASTEXITCODE -ne 0) { Write-Error "Installer build failed"; exit 1 }

$installerName = "KType-v$versionNum-x64-setup.exe"
$installerPath = "build\$installerName"
if (-not (Test-Path $installerPath)) {
    Write-Error "Installer not found at $installerPath"
    exit 1
}
# Sign installer
if ($canSign) {
    Write-Host "Signing installer..." -ForegroundColor Yellow
    & $signtool sign /a /s My /n $certName /fd sha256 /td sha256 /tr http://timestamp.digicert.com $installerPath
    if ($LASTEXITCODE -ne 0) { Write-Host "Installer signing failed (continuing)" -ForegroundColor DarkYellow }
    else { Write-Host "Installer signed" -ForegroundColor Green }
}

$size = [math]::Round((Get-Item $installerPath).Length / 1MB, 1)
Write-Host "Created $installerName ($size MB)" -ForegroundColor Green

# --- Publish to GitHub ---
Write-Host "`n[4/4] Publishing to GitHub..." -ForegroundColor Yellow

if ($Overwrite) {
    try {
        $existing = Invoke-GitHubApi -Method Get -Uri "https://api.github.com/repos/$repo/releases/tags/$Version"
        Invoke-GitHubApi -Method Delete -Uri "https://api.github.com/repos/$repo/releases/$($existing.id)" | Out-Null
        Write-Host "Deleted existing release $Version" -ForegroundColor DarkYellow
    } catch {}
    git tag -d $Version 2>$null; $null = $LASTEXITCODE
    git push origin --delete $Version 2>$null; $null = $LASTEXITCODE
}

$releaseBody = @{
    tag_name = $Version
    name = "KType $Version"
    body = "KType $Version - Vietnamese Telex IME for Windows`n`nRun the installer (as admin) to install. Previous versions are automatically replaced."
    draft = [bool]$Draft
    prerelease = $false
} | ConvertTo-Json -Compress

$release = Invoke-GitHubApi -Method Post -Uri "https://api.github.com/repos/$repo/releases" -Body $releaseBody

# Upload installer
$uploadUrl = "https://uploads.github.com/repos/$repo/releases/$($release.id)/assets?name=$installerName"
$fileBytes = [System.IO.File]::ReadAllBytes((Resolve-Path $installerPath))
$headers = @{
    'Authorization' = "token $(Get-GitHubToken)"
    'Accept' = 'application/vnd.github+json'
    'Content-Type' = 'application/octet-stream'
}
$asset = Invoke-RestMethod -Uri $uploadUrl -Method Post -Headers $headers -Body $fileBytes

Write-Host "`nRelease $Version published!" -ForegroundColor Green
Write-Host "URL: $($release.html_url)" -ForegroundColor Cyan
Write-Host "Download: $($asset.browser_download_url)" -ForegroundColor Cyan
