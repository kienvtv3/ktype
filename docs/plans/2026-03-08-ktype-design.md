# KType Design

## Mục tiêu

Trình gõ tiếng Việt Telex tối giản cho Windows, dùng TSF (Text Services Framework) thay vì fake backspace. Hoạt động tốt với mọi app kể cả Claude Code.

## Quyết định công nghệ

- **Ngôn ngữ**: C++ (ATL/COM)
- **Build**: Visual Studio / MSBuild
- **Framework**: Windows TSF
- **Installer**: Làm sau (WiX hoặc NSIS)

## Kiến trúc

2 thành phần:
- `ktype.dll` — TSF text service (C++, ATL/COM), core duy nhất
- `ktype-settings.exe` — app cấu hình (làm sau)

3 layer trong DLL:

```
┌─────────────────────────┐
│  TSF Integration Layer  │  TextService, ContextManager, KeyEventSink
│  (COM/ATL)              │  Đăng ký profile, xử lý lifecycle
├─────────────────────────┤
│  Composition Manager    │  Quản lý composition, commit/cancel
│                         │  Handle Enter/Tab/Esc/focus loss → commit
├─────────────────────────┤
│  Telex Engine           │  Pure C++, không Windows dependency
│  (portable)             │  Xử lý logic gõ Telex, tone, dấu
└─────────────────────────┘
```

## Hành vi chính

| Tình huống | Hành vi |
|---|---|
| `Win+Space` sang KType | Bật gõ tiếng Việt ngay, không cần toggle thêm |
| `Win+Space` sang ENG | Gõ tiếng Anh |
| Gõ `_`, space, dấu câu | Commit word hiện tại, clear buffer, bắt đầu word mới |
| Enter, Tab, Esc, mất focus | Commit buffer rồi gửi phím xuống app |
| Gõ `w` | Output `w` (không phải `ư`), theo logic quen thuộc của VietType |
| Bỏ dấu | Hỗ trợ cả kiểu mới (oà) và cũ (òa), cấu hình được |

## Settings (giai đoạn sau)

- Kiểu bỏ dấu: mới (oà, uỳ) hay cũ (òa, úy)
- Bật/tắt spell check
- Lưu vào registry, app Settings riêng mở từ Start Menu

## Tham khảo code

- **VietType** (`../viettype/`): TSF integration pattern, Telex engine structure, registration
- **OpenKey** (`../openkey/`): Telex logic chi tiết (tone placement, spell check patterns)

## Không làm (YAGNI)

- Không VNI, chỉ Telex
- Không macro, không quick Telex
- Không auto-correct nâng cao
- Không system tray icon
- Không multi-encoding (chỉ Unicode)
