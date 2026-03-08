#define MyAppName "KType"
#define MyAppPublisher "KType"
#define MyAppURL "https://github.com/kienvtv3/ktype"

; Version is passed via /DMyAppVersion=x.y.z from command line
#ifndef MyAppVersion
  #define MyAppVersion "0.1.0"
#endif

[Setup]
AppId={{E8F2A1B3-4C5D-6E7F-8A9B-0C1D2E3F4A5B}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
DefaultDirName={commonpf}\{#MyAppName}
DisableProgramGroupPage=yes
OutputDir=..\build
OutputBaseFilename=KType-v{#MyAppVersion}-x64-setup
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0
PrivilegesRequired=admin
UninstallDisplayName={#MyAppName}
UninstallDisplayIcon={app}\KType.dll
WizardStyle=modern
UsePreviousAppDir=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "..\build\x64\Release\KType.dll"; DestDir: "{app}"; Flags: ignoreversion regserver
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion

[Run]
; After DLL registration, remove the default Vietnamese keyboard that Windows auto-adds
Filename: "powershell.exe"; \
  Parameters: "-NoProfile -ExecutionPolicy Bypass -Command ""$l = Get-WinUserLanguageList; foreach ($lang in $l) {{ if ($lang.LanguageTag -like 'vi*') {{ $remove = @(); foreach ($tip in $lang.InputMethodTips) {{ if ($tip -like '042A:0000042A*') {{ $remove += $tip }} }}; foreach ($t in $remove) {{ $lang.InputMethodTips.Remove($t) | Out-Null }} }} }}; Set-WinUserLanguageList $l -Force"""; \
  Flags: runhidden waituntilterminated; \
  StatusMsg: "Configuring keyboard layout..."

[Code]
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  ResultCode: Integer;
begin
  if CurUninstallStep = usUninstall then
  begin
    // Unregister the COM server before removing files
    Exec('regsvr32.exe', '/u /s "' + ExpandConstant('{app}\KType.dll') + '"',
         '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  end;
end;
