# KType pre-uninstall: remove Vietnamese language entirely
# Restores system to pre-install state (only English)

# Remove keyboard layout substitution
$subKey = 'HKCU:\Keyboard Layout\Substitutes'
if (Test-Path $subKey) {
    Remove-ItemProperty -Path $subKey -Name '0000042a' -ErrorAction SilentlyContinue
}

$l = Get-WinUserLanguageList

# Remove Vietnamese language completely
$vi = $l | Where-Object { $_.LanguageTag -like 'vi*' }
if ($vi) {
    $l.Remove($vi) | Out-Null
}

Set-WinUserLanguageList $l -Force
