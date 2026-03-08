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

$l = Get-WinUserLanguageList

# Step 1: Add Vietnamese language if not present
$vi = $l | Where-Object { $_.LanguageTag -like 'vi*' }
if (-not $vi) {
    $l.Add('vi')
    Set-WinUserLanguageList $l -Force
    Start-Sleep -Seconds 2
    $l = Get-WinUserLanguageList
}

# Step 2: For the Vietnamese language entry, add KType and remove everything else
foreach ($lang in $l) {
    if ($lang.LanguageTag -like 'vi*') {
        # Remove all existing Vietnamese keyboards/IMEs
        $lang.InputMethodTips.Clear()
        # Add only KType
        $lang.InputMethodTips.Add($ktypeTip)
    }
}

Set-WinUserLanguageList $l -Force
