# Create self-signed code signing certificate for KType
# Run ONCE as admin. After this, release.ps1 will auto-sign.
#
# Usage (elevated PowerShell):
#   .\create-cert.ps1

$ErrorActionPreference = "Stop"

$certName = "Kien Vu (KType)"
$storePath = "Cert:\CurrentUser\My"

# Check if cert already exists
$existing = Get-ChildItem $storePath -CodeSigningCert | Where-Object { $_.Subject -eq "CN=$certName" }
if ($existing) {
    Write-Host "Certificate already exists: $($existing.Thumbprint)" -ForegroundColor Yellow
    Write-Host "To recreate, delete it first from certmgr.msc" -ForegroundColor DarkYellow
    exit 0
}

# Create self-signed code signing certificate (valid 10 years)
$cert = New-SelfSignedCertificate `
    -Type CodeSigningCert `
    -Subject "CN=$certName" `
    -CertStoreLocation $storePath `
    -NotAfter (Get-Date).AddYears(10) `
    -KeyUsage DigitalSignature `
    -FriendlyName "KType Code Signing"

Write-Host "Certificate created: $($cert.Thumbprint)" -ForegroundColor Green

# Export and import to Trusted Root + Trusted Publishers so Windows trusts it
$certPath = "$env:TEMP\ktype-codesign.cer"
Export-Certificate -Cert $cert -FilePath $certPath | Out-Null

# Import to Trusted Root CA (requires elevation)
Import-Certificate -FilePath $certPath -CertStoreLocation "Cert:\LocalMachine\Root" | Out-Null
Write-Host "Added to Trusted Root Certification Authorities" -ForegroundColor Green

# Import to Trusted Publishers
Import-Certificate -FilePath $certPath -CertStoreLocation "Cert:\LocalMachine\TrustedPublisher" | Out-Null
Write-Host "Added to Trusted Publishers" -ForegroundColor Green

Remove-Item $certPath

Write-Host "`nDone! KType builds will now be trusted on this machine." -ForegroundColor Cyan
Write-Host "release.ps1 will auto-sign using this certificate." -ForegroundColor Cyan
