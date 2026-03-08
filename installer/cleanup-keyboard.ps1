# KType pre-uninstall: remove KType from language list, restore Vietnamese Telex
# Run before COM unregistration

$ktypeTip = '042A:{7E8F1A2B-3C4D-5E6F-A1B2-C3D4E5F67890}{8F9A2B3C-4D5E-6F7A-B2C3-D4E5F6789012}'
$telexTip = '042A:{C2CB2CF0-AF47-413E-9780-8BC3A3C16068}{5FB02EC5-0A77-4684-B4FA-DEF8A2195628}'

$l = Get-WinUserLanguageList

foreach ($lang in $l) {
    if ($lang.LanguageTag -like 'vi*') {
        # Remove KType
        $lang.InputMethodTips.Remove($ktypeTip) | Out-Null
        # Remove default keyboard if present
        $toRemove = @()
        foreach ($tip in $lang.InputMethodTips) {
            if ($tip -like '042A:0000042A*') { $toRemove += $tip }
        }
        foreach ($t in $toRemove) { $lang.InputMethodTips.Remove($t) | Out-Null }
        # Restore Vietnamese Telex so user still has Vietnamese input
        if (-not ($lang.InputMethodTips -contains $telexTip)) {
            $lang.InputMethodTips.Add($telexTip)
        }
    }
}

Set-WinUserLanguageList $l -Force
