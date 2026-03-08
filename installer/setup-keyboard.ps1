# KType post-install: configure Vietnamese language and keyboard
# Run after DLL registration (regsvr32)

$ktypeTip = '042A:{7E8F1A2B-3C4D-5E6F-A1B2-C3D4E5F67890}{8F9A2B3C-4D5E-6F7A-B2C3-D4E5F6789012}'

# Step 0: Map Vietnamese keyboard layout to US English (VietType behavior)
# Without this, keys KType doesn't eat go through the Vietnamese layout
# which maps number keys to diacritics (ă â ê ô + tone marks)
$subKey = 'HKCU:\Keyboard Layout\Substitutes'
if (-not (Test-Path $subKey)) {
    New-Item -Path $subKey -Force | Out-Null
}
Set-ItemProperty -Path $subKey -Name '0000042a' -Value '00000409'

# Step 1: Use InstallLayoutOrTip API — more reliable than Set-WinUserLanguageList
Add-Type @"
using System;
using System.Runtime.InteropServices;
public class InputApi {
    [DllImport("input.dll", CharSet = CharSet.Unicode)]
    public static extern bool InstallLayoutOrTip(string psz, uint flags);
}
"@

# Ensure Vietnamese language exists
$l = Get-WinUserLanguageList
$vi = $l | Where-Object { $_.LanguageTag -like 'vi*' }
if (-not $vi) {
    $l.Add('vi')
    Set-WinUserLanguageList $l -Force
    Start-Sleep -Seconds 2
}

# Install KType as Vietnamese IME
[InputApi]::InstallLayoutOrTip($ktypeTip, 0) | Out-Null

# Remove default Vietnamese keyboard layout that Windows auto-adds
# (it maps number keys to diacritics, conflicting with KType)
[InputApi]::InstallLayoutOrTip('042A:0000042A', 1) | Out-Null

# Clean up: ensure only KType under Vietnamese
$l = Get-WinUserLanguageList
foreach ($lang in $l) {
    if ($lang.LanguageTag -like 'vi*') {
        # Remove any non-KType keyboards
        $toRemove = @($lang.InputMethodTips | Where-Object { $_ -ne $ktypeTip })
        foreach ($tip in $toRemove) {
            $lang.InputMethodTips.Remove($tip) | Out-Null
        }
        # Ensure KType is present
        if ($lang.InputMethodTips -notcontains $ktypeTip) {
            $lang.InputMethodTips.Add($ktypeTip)
        }
    }
}
Set-WinUserLanguageList $l -Force
