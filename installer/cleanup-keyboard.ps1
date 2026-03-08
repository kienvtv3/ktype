# KType pre-uninstall: remove Vietnamese language entirely
# Restores system to pre-install state (only English)

$l = Get-WinUserLanguageList

# Remove Vietnamese language completely
$vi = $l | Where-Object { $_.LanguageTag -like 'vi*' }
if ($vi) {
    $l.Remove($vi) | Out-Null
}

Set-WinUserLanguageList $l -Force
