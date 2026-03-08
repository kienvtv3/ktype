#define MyAppName "KType"
#define MyAppPublisher "KType"
#define MyAppURL "https://github.com/kienvtv3/ktype"

; Version is passed via /DMyAppVersion=x.y.z from command line
#ifndef MyAppVersion
  #define MyAppVersion "0.1.0"
#endif

; KType TSF identifiers (must match globals.cpp)
#define KTypeCLSID   "{7E8F1A2B-3C4D-5E6F-A1B2-C3D4E5F67890}"
#define KTypeProfile "{8F9A2B3C-4D5E-6F7A-B2C3-D4E5F6789012}"
#define KTypeTip     "042A:{7E8F1A2B-3C4D-5E6F-A1B2-C3D4E5F67890}{8F9A2B3C-4D5E-6F7A-B2C3-D4E5F6789012}"

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
Source: "setup-keyboard.ps1"; DestDir: "{app}"; Flags: ignoreversion
Source: "cleanup-keyboard.ps1"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion

[Run]
; After DLL registration: add KType to Vietnamese language, remove Telex
Filename: "powershell.exe"; \
  Parameters: "-NoProfile -ExecutionPolicy Bypass -File ""{app}\setup-keyboard.ps1"""; \
  Flags: runhidden waituntilterminated; \
  StatusMsg: "Configuring keyboard layout..."

[UninstallDelete]
; Force-remove app directory in case DLL was locked during uninstall
Type: filesandordirs; Name: "{app}"

[Code]
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  ResultCode: Integer;
begin
  if CurUninstallStep = usUninstall then
  begin
    // Remove KType from language list, restore Vietnamese Telex
    Exec('powershell.exe',
         '-NoProfile -ExecutionPolicy Bypass -File "' + ExpandConstant('{app}\cleanup-keyboard.ps1') + '"',
         '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  end;
end;
