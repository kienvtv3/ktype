# Delivery & Testing Design

## Build & Release Flow

```
Remote (Claude build)
  │
  ├─ msbuild Release x64 → KTypeATL.dll
  ├─ Inno Setup compile  → KType-Setup-x64.exe
  │
  └─ gh release upload   → GitHub Releases
                               │
Local (user test)              │
  └─ Download installer ←─────┘
     → Run KType-Setup-x64.exe
     → Tự register DLL, thêm vào language
     → Win+Space → test gõ tiếng Việt
```

## GitHub Release Strategy

- **Dev builds**: Tag `nightly`, override mỗi lần build mới. URL cố định: `github.com/kienvtv3/ktype/releases/tag/nightly`
- **Stable releases**: Khi ổn định, tag version `v0.1.0`, `v0.2.0`, etc. Không override.
- Mỗi release có: `KType-Setup-x64.exe` + changelog ngắn

## Installer (Inno Setup)

File `.iss` script:
1. Copy `KTypeATL.dll` vào `C:\Program Files\KType\`
2. Chạy `regsvr32 /s KTypeATL.dll` để register COM/TSF
3. Uninstall: `regsvr32 /u /s KTypeATL.dll` rồi xoá file

## Quy trình test

1. Build + push release `nightly`
2. User download installer từ GitHub releases
3. Chạy installer (cần admin)
4. Test gõ tiếng Việt, báo bug
5. Fix, push lại `nightly`, user uninstall cũ → cài mới
