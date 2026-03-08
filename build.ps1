# KType build script
# Usage:
#   .\build.ps1                      # Debug build
#   .\build.ps1 -Config Release      # Release build
#   .\build.ps1 -Clean               # Clean + build

param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Debug",
    [switch]$Clean
)

# Import VS environment
$vsPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools"
$vcvarsall = "$vsPath\VC\Auxiliary\Build\vcvarsall.bat"

if (-not (Test-Path $vcvarsall)) {
    Write-Error "VS Build Tools not found at $vsPath"
    exit 1
}

# Load vcvarsall environment into PowerShell
cmd /c "`"$vcvarsall`" x64 >nul 2>&1 && set" | ForEach-Object {
    if ($_ -match "^(.+?)=(.*)$") {
        [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2], "Process")
    }
}

$target = if ($Clean) { "/t:Clean;Build" } else { "/t:Build" }

msbuild KType.sln /p:Configuration=$Config /p:Platform=x64 $target /m /v:minimal
