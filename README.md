# KType

Bộ gõ tiếng Việt tối giản cho Windows, sử dụng Text Services Framework (TSF).

A minimal Vietnamese IME for Windows, built on TSF — the same framework used by Microsoft's own input methods.

## Tại sao lại cần KType? / Why KType?

Các bộ gõ phổ biến như Unikey hay EVKey hoạt động bằng cách giả lập phím Backspace để xoá và gõ lại ký tự. Cách này gây lỗi trên nhiều ứng dụng hiện đại — đặc biệt là **các terminal như Claude Code, Windows Terminal, VS Code terminal** — nơi mà phím Backspace bị xử lý khác đi, khiến việc gõ tiếng Việt bị sai hoặc không hoạt động.

KType sử dụng TSF (Text Services Framework), cơ chế composition giống các IME của Microsoft (Nhật, Hàn, Trung). Văn bản được soạn trực tiếp trong ứng dụng mà không cần giả lập phím.

Popular Vietnamese IMEs like Unikey and EVKey work by simulating Backspace keystrokes to replace text. This approach breaks in many modern applications — especially **terminals like Claude Code, Windows Terminal, and VS Code integrated terminal** — where Backspace is handled differently, causing Vietnamese input to malfunction or fail entirely.

KType uses TSF (Text Services Framework), the same composition mechanism used by Microsoft's own IMEs (Japanese, Korean, Chinese). Text is composed directly in the application without keystroke simulation.

| | Fake Backspace (Unikey/EVKey) | TSF (KType) |
|---|---|---|
| Soạn văn bản | Giả lập Backspace + gõ lại | Soạn trực tiếp tại chỗ |
| Terminal / Claude Code | Thường bị lỗi | Hoạt động bình thường |
| Xem trước khi gõ | Không có | Gạch chân khi đang soạn |
| Phần mềm bảo mật | Có thể bị đánh dấu keylogger | API chuẩn Windows |

## Tính năng / Features

### Phương thức gõ / Input Method
- **Telex đầy đủ**: aa→â, aw→ă, ow→ơ, uw→ư, ee→ê, oo→ô, dd→đ
- **6 dấu thanh**: s (sắc), f (huyền), r (hỏi), x (ngã), j (nặng), z (xoá dấu)
- **Thay dấu linh hoạt**: gõ dấu khác sẽ thay thế dấu hiện tại, gõ cùng dấu sẽ xoá
- **Phím W đa năng**: ư (uw), ơ (ow), ươ (uow), ă chỉ trong wa

### Chính tả / Orthography
- **Kiểm tra cấu trúc âm tiết**: phụ âm đầu (C1) + nguyên âm (V) + phụ âm cuối (C2)
- **Vị trí dấu thanh chuẩn**: tự động đặt dấu đúng vị trí theo quy tắc tiếng Việt
- **Tuỳ chọn kiểu dấu**: kiểu mới "hoà" (mặc định) hoặc kiểu cũ "hòa"
- **Ràng buộc thanh điệu**: âm cuối c/ch/k/p/t chỉ chấp nhận sắc hoặc nặng
- **Đầy đủ phụ âm ghép**: ch, gh, gi, kh, ng, ngh, nh, ph, qu, th, tr
- **Nhận diện nguyên âm ghép**: iê, uô, ươ, oa, oe, uy, ai, ao, au, ...

### Engine
- **Soạn thảo thời gian thực**: xử lý từng phím, phản hồi tức thì
- **Xem trước (preview)**: hiển thị kết quả khi đang gõ mà chưa cần commit
- **Backspace thông minh**: xoá ký tự cuối và tính lại toàn bộ từ
- **Giữ nguyên hoa/thường**: HOÀ, Hoà, hoà — theo đúng cách bạn gõ
- **Viết tắt**: dd→đ trong từ viết tắt (vd: "ddc"→"đc", "qdd"→"qđ")
- **264 test cases** bao phủ đầy đủ các trường hợp gõ tiếng Việt

### Cài đặt / Installation
- **Tự động thêm ngôn ngữ**: thêm Tiếng Việt vào Windows nếu chưa có
- **Đặt mặc định**: tự đặt KType làm bộ gõ cho Tiếng Việt
- **Hỗ trợ UWP**: hoạt động trong cả ứng dụng Windows Store (Settings, v.v.)
- **Cập nhật an toàn**: tự xử lý khi DLL đang bị khoá bởi ứng dụng khác

## Cài đặt / Installation

Tải file **KType-vX.Y.Z-x64-setup.exe** từ [Releases](https://github.com/kienvtv3/ktype/releases), chạy với quyền Admin.

Download **KType-vX.Y.Z-x64-setup.exe** from [Releases](https://github.com/kienvtv3/ktype/releases) and run as Administrator.

Chuyển đổi giữa tiếng Anh và tiếng Việt bằng **Win + Space**.

> **Lưu ý**: Windows SmartScreen có thể cảnh báo vì KType chưa có chữ ký số (code signing certificate). Đây là phần mềm mã nguồn mở — bạn có thể kiểm tra mã nguồn hoặc tự build từ source.

## Xác minh và bảo mật / Verification & Security

KType được cung cấp nguyên trạng, không bao gồm bất cứ bảo hành hay bảo đảm nào. Bạn tự chấp nhận mọi rủi ro và trách nhiệm khi sử dụng KType.

KType is provided as-is, without any warranty or guarantee. You accept all risks and responsibilities when using KType.

Bạn có thể kiểm chứng độ an toàn bằng những cách sau:
- Chỉ tải KType từ trang chính thức: https://github.com/kienvtv3/ktype
- Kiểm tra mã hash của file tải về
- Kiểm tra toàn bộ mã nguồn — KType là mã nguồn mở hoàn toàn

```powershell
# Kiểm tra hash file cài đặt
Get-FileHash KType-v*-setup.exe -Algorithm SHA256
```

## Build từ mã nguồn / Building from Source

Yêu cầu Visual Studio Build Tools 2022+ với C++ Desktop workload và ATL.

```powershell
$msbuild = "path\to\MSBuild.exe"

# Build
& $msbuild KType.sln /p:Configuration=Release /p:Platform=x64

# Chạy tests
.\tests\build\x64\Release\tests.exe
```

## Cấu trúc dự án / Project Structure

```
src/engine/    # Telex engine — C++ thuần, không phụ thuộc Windows
src/tsf/       # Tích hợp TSF — COM/ATL DLL
tests/         # 264 tests (8 files)
installer/     # Inno Setup script
```

## Lời cảm ơn / Acknowledgments

- [VietType](https://github.com/dinhngtu/VietType) — KType có tham khảo một số ý tưởng xử lý âm tiết tiếng Việt từ VietType.

## Giấy phép / License

GPL-3.0 — xem [LICENSE](LICENSE).
