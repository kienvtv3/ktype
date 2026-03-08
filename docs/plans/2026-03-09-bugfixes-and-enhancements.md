# Bugfixes & Enhancements Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Fix 4 issues: space double-press, uninstall folder cleanup, towupper for Vietnamese, English word collision.

**Architecture:** Task 1 fixes TSF key handling by injecting the triggering character after commit (instead of trying to un-eat). Task 2 adds Inno Setup cleanup directive. Task 3 adds a custom uppercase mapping table for Vietnamese diacritics. Task 4 adds a minimal English word list to reject common collisions at Commit time.

**Tech Stack:** C++20, Windows TSF API, Inno Setup

---

### Task 1: Fix space double-press (TSF key eating)

**Root cause:** `OnTestKeyDown` sets `pfEaten=TRUE` for space when composition is active. `OnKeyDown` commits and sets `pfEaten=FALSE`, but the key was already eaten by OnTestKeyDown — the application never sees it. User must press space twice.

**Fix:** In `OnKeyDown`, commit composition AND inject the triggering character into the document ourselves. Keep `pfEaten=TRUE` (we handle the full lifecycle).

**Files:**
- Modify: `src/tsf/context.h` — add `CommitAndInsertChar` method
- Modify: `src/tsf/context.cpp` — implement `CommitAndInsertChar`
- Modify: `src/tsf/key_handler.cpp:124-136` — call new method instead of just commit

**Step 1: Add `CommitAndInsertChar` to Context**

In `src/tsf/context.h`, add after `CommitComposition`:
```cpp
HRESULT CommitAndInsertChar(TfEditCookie ec, wchar_t ch);
```

In `src/tsf/context.cpp`, add after `CommitComposition`:
```cpp
HRESULT Context::CommitAndInsertChar(TfEditCookie ec, wchar_t ch) {
    // First commit any pending composition
    HRESULT hr = CommitComposition(ec);
    if (FAILED(hr)) return hr;

    // Now insert the triggering character at the current selection
    ATL::CComPtr<ITfInsertAtSelection> insertAtSel;
    hr = _tfContext->QueryInterface(&insertAtSel);
    if (FAILED(hr)) return hr;

    wchar_t buf[2] = { ch, 0 };
    ATL::CComPtr<ITfRange> range;
    hr = insertAtSel->InsertTextAtSelection(ec, 0, buf, 1, &range);
    return hr;
}
```

**Step 2: Update `OnKeyDown` non-alphabetic char path**

In `src/tsf/key_handler.cpp`, replace lines 124-136:
```cpp
// Non-alphabetic printable char with pending input → commit + inject char
if (ctx->HasPendingInput()) {
    auto* session = new EditSession([ctx, ch](TfEditCookie ec) -> HRESULT {
        return ctx->CommitAndInsertChar(ec, ch);
    });
    HRESULT hrSession;
    tfContext->RequestEditSession(_clientId, session,
        TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hrSession);
    session->Release();
    *pfEaten = TRUE;  // We handled the full lifecycle (commit + inject)
    return S_OK;
}
```

**Step 3: Build and test manually**

```powershell
$msbuild = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
& $msbuild KType.sln /p:Configuration=Release /p:Platform=x64 /nologo /v:minimal
```

Manual test: register DLL, open file rename dialog, type Vietnamese then space. Space should appear immediately without double-press.

**Step 4: Commit**

```bash
git add src/tsf/context.h src/tsf/context.cpp src/tsf/key_handler.cpp
git commit -m "fix: space key no longer requires double-press after commit"
```

---

### Task 2: Fix uninstall folder not deleted

**Root cause:** The `[Code]` section calls `regsvr32 /u` manually before Inno Setup's own uninstall. The DLL may be locked by running processes (TSF DLLs are loaded into every app). Inno can't delete locked files, leaving the folder behind.

**Fix:** Add `[UninstallDelete]` section to force-cleanup, and remove the manual `regsvr32 /u` call (let Inno handle it via the `regserver` flag).

**Files:**
- Modify: `installer/ktype.iss`

**Step 1: Update ktype.iss**

Remove manual `regsvr32 /u` from `[Code]` section (Inno's `regserver` flag handles this).
Add `[UninstallDelete]` section to clean up any remaining files.

Replace `[Code]` section with:
```pascal
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
```

**Step 2: Commit**

```bash
git add installer/ktype.iss
git commit -m "fix: ensure KType folder is deleted on uninstall"
```

---

### Task 3: Fix `towupper` for Vietnamese diacritics

**Root cause:** Windows `towupper()` doesn't handle Vietnamese characters like â→Â, đ→Đ, ế→Ế, etc. The engine's `ApplyCases()` uses `towupper()`, so uppercase Vietnamese text has lowercase diacritics.

**Fix:** Add a custom `VnToUpper(wchar_t)` function with a lookup table covering all 134 Vietnamese lowercase→uppercase mappings.

**Files:**
- Modify: `src/engine/telex_data.h` — add `VnToUpper` function
- Modify: `src/engine/telex_engine.cpp:356-401` — replace `towupper` calls
- Modify: `tests/test_case.cpp` — fix test expectations
- Modify: `tests/test_viettype.cpp` — fix uppercase test expectations

**Step 1: Write the failing tests**

In `tests/test_case.cpp`, update `test_all_upper` and `test_upper_dd`:
```cpp
// All uppercase
bool test_all_upper() {
    TelexEngine e;
    e.PushChar(L'V'); e.PushChar(L'I'); e.PushChar(L'E');
    e.PushChar(L'E'); e.PushChar(L'J'); e.PushChar(L'T');
    e.Commit();
    ASSERT_WSTR_EQ(e.Retrieve(), L"VI\x1ec6T");  // VIỆT (uppercase ệ = Ệ = U+1EC6)
    return true;
}

bool test_upper_dd() {
    TelexEngine e;
    e.PushChar(L'D'); e.PushChar(L'D'); push(e, "a");
    e.Commit();
    ASSERT_WSTR_EQ(e.Retrieve(), L"\x0110" L"a");  // Đa (Đ = U+0110)
    return true;
}
```

In `tests/test_viettype.cpp`, update uppercase tests:
```cpp
bool test_vt_AAN()  { ASSERT_WSTR_EQ(commit("AAn"), L"\x00c2n"); return true; }   // Ân
bool test_vt_Aan()  { ASSERT_WSTR_EQ(commit("Aan"), L"\x00c2n"); return true; }   // Ân
bool test_vt_DDi()  { ASSERT_WSTR_EQ(commit("DDi"), L"\x0110i"); return true; }   // Đi
bool test_vt_Ddi()  { ASSERT_WSTR_EQ(commit("Ddi"), L"\x0110i"); return true; }   // Đi
```

Run tests — these should FAIL (towupper doesn't produce correct results).

**Step 2: Implement `VnToUpper` in `telex_data.h`**

Add after `IsConsonant()`:
```cpp
// Custom Vietnamese uppercase mapping (towupper fails for Vietnamese diacritics)
inline wchar_t VnToUpper(wchar_t c) {
    // ASCII fast path
    if (c >= L'a' && c <= L'z') return c - 32;

    // Vietnamese diacritic mappings (lowercase → uppercase)
    switch (c) {
    // đ → Đ
    case L'\x0111': return L'\x0110';
    // ă, ắ, ằ, ẳ, ẵ, ặ
    case L'\x0103': return L'\x0102';
    case L'\x1eaf': return L'\x1eae';
    case L'\x1eb1': return L'\x1eb0';
    case L'\x1eb3': return L'\x1eb2';
    case L'\x1eb5': return L'\x1eb4';
    case L'\x1eb7': return L'\x1eb6';
    // â, ấ, ầ, ẩ, ẫ, ậ
    case L'\x00e2': return L'\x00c2';
    case L'\x1ea5': return L'\x1ea4';
    case L'\x1ea7': return L'\x1ea6';
    case L'\x1ea9': return L'\x1ea8';
    case L'\x1eab': return L'\x1eaa';
    case L'\x1ead': return L'\x1eac';
    // á, à, ả, ã, ạ
    case L'\x00e1': return L'\x00c1';
    case L'\x00e0': return L'\x00c0';
    case L'\x1ea3': return L'\x1ea2';
    case L'\x00e3': return L'\x00c3';
    case L'\x1ea1': return L'\x1ea0';
    // ê, ế, ề, ể, ễ, ệ
    case L'\x00ea': return L'\x00ca';
    case L'\x1ebf': return L'\x1ebe';
    case L'\x1ec1': return L'\x1ec0';
    case L'\x1ec3': return L'\x1ec2';
    case L'\x1ec5': return L'\x1ec4';
    case L'\x1ec7': return L'\x1ec6';
    // é, è, ẻ, ẽ, ẹ
    case L'\x00e9': return L'\x00c9';
    case L'\x00e8': return L'\x00c8';
    case L'\x1ebb': return L'\x1eba';
    case L'\x1ebd': return L'\x1ebc';
    case L'\x1eb9': return L'\x1eb8';
    // í, ì, ỉ, ĩ, ị
    case L'\x00ed': return L'\x00cd';
    case L'\x00ec': return L'\x00cc';
    case L'\x1ec9': return L'\x1ec8';
    case L'\x0129': return L'\x0128';
    case L'\x1ecb': return L'\x1eca';
    // ô, ố, ồ, ổ, ỗ, ộ
    case L'\x00f4': return L'\x00d4';
    case L'\x1ed1': return L'\x1ed0';
    case L'\x1ed3': return L'\x1ed2';
    case L'\x1ed5': return L'\x1ed4';
    case L'\x1ed7': return L'\x1ed6';
    case L'\x1ed9': return L'\x1ed8';
    // ơ, ớ, ờ, ở, ỡ, ợ
    case L'\x01a1': return L'\x01a0';
    case L'\x1edb': return L'\x1eda';
    case L'\x1edd': return L'\x1edc';
    case L'\x1edf': return L'\x1ede';
    case L'\x1ee1': return L'\x1ee0';
    case L'\x1ee3': return L'\x1ee2';
    // ó, ò, ỏ, õ, ọ
    case L'\x00f3': return L'\x00d3';
    case L'\x00f2': return L'\x00d2';
    case L'\x1ecf': return L'\x1ece';
    case L'\x00f5': return L'\x00d5';
    case L'\x1ecd': return L'\x1ecc';
    // ư, ứ, ừ, ử, ữ, ự
    case L'\x01b0': return L'\x01af';
    case L'\x1ee9': return L'\x1ee8';
    case L'\x1eeb': return L'\x1eea';
    case L'\x1eed': return L'\x1eec';
    case L'\x1eef': return L'\x1eee';
    case L'\x1ef1': return L'\x1ef0';
    // ú, ù, ủ, ũ, ụ
    case L'\x00fa': return L'\x00da';
    case L'\x00f9': return L'\x00d9';
    case L'\x1ee7': return L'\x1ee6';
    case L'\x0169': return L'\x0168';
    case L'\x1ee5': return L'\x1ee4';
    // ý, ỳ, ỷ, ỹ, ỵ
    case L'\x00fd': return L'\x00dd';
    case L'\x1ef3': return L'\x1ef2';
    case L'\x1ef7': return L'\x1ef6';
    case L'\x1ef9': return L'\x1ef8';
    case L'\x1ef5': return L'\x1ef4';
    default: return (wchar_t)towupper(c);  // fallback for non-Vietnamese
    }
}
```

**Step 3: Replace `towupper` calls in `telex_engine.cpp`**

In `ApplyCases()`, replace all 3 occurrences of:
```cpp
result[outputIdx] = (wchar_t)towupper(result[outputIdx]);
```
with:
```cpp
result[outputIdx] = TelexData::VnToUpper(result[outputIdx]);
```

Also in `Replay()` (~line 523), replace:
```cpp
c = (wchar_t)towupper(c);
```
with:
```cpp
c = TelexData::VnToUpper(c);
```

**Step 4: Build and run tests**

```powershell
& $msbuild tests\tests.vcxproj /p:Configuration=Debug /p:Platform=x64 /nologo /v:minimal
.\tests\build\x64\Debug\tests.exe
```

All tests should pass with correct uppercase Vietnamese.

**Step 5: Commit**

```bash
git add src/engine/telex_data.h src/engine/telex_engine.cpp tests/test_case.cpp tests/test_viettype.cpp
git commit -m "fix: correct uppercase for Vietnamese diacritics (custom VnToUpper)"
```

---

### Task 4: English word collision detection

**Root cause:** Telex keys (s, f, r, x, j, z, w) overlap with English consonants. Typing "test" → "tét", "docs" → "dóc". VietType uses a dictionary (optimize_multilang). Unikey uses phonotactic rules (autoNonVnRestore).

**Fix:** Add a minimal English word list (~130 common collision words from VietType's `confuse.txt`) and reject matches at Commit time. This is VietType's Level 1 optimization.

**Files:**
- Modify: `src/engine/telex.h` — add `optimize_multilang` config
- Create: `src/engine/wordlist.h` — English word list
- Modify: `src/engine/telex_engine.cpp` — check word list in Commit
- Modify: `tests/test_viettype.cpp` — add English rejection tests

**Step 1: Write the failing tests**

In `tests/test_viettype.cpp`, add:
```cpp
// English word collision: optimize_multilang rejects common English words
bool test_vt_english_test() {
    TelexConfig cfg;
    cfg.optimize_multilang = 1;
    ASSERT_WSTR_EQ(commit("test", cfg), L"test");  // "test" rejected, raw output
    return true;
}
bool test_vt_english_just() {
    TelexConfig cfg;
    cfg.optimize_multilang = 1;
    ASSERT_WSTR_EQ(commit("just", cfg), L"just");
    return true;
}
bool test_vt_english_off() {
    TelexConfig cfg;
    cfg.optimize_multilang = 0;
    ASSERT_WSTR_EQ(commit("test", cfg), L"t\x00e9t");  // optimize off → "tét"
    return true;
}
```

Run tests — should FAIL (optimize_multilang not implemented yet).

**Step 2: Add config option**

In `src/engine/telex.h`, add to `TelexConfig`:
```cpp
int optimize_multilang = 1;  // 0=off, 1=basic English word rejection
```

**Step 3: Create word list**

Create `src/engine/wordlist.h` with words from VietType's `confuse.txt` plus common additions:
```cpp
#pragma once
#include <unordered_set>
#include <string>

namespace KType {

// English words that commonly collide with Telex input
// Source: VietType confuse.txt + common additions
inline const std::unordered_set<std::wstring>& EnglishWordList() {
    static const std::unordered_set<std::wstring> words = {
        // From VietType confuse.txt (DoubleTone pattern)
        L"airs", L"arms", L"avengers", L"awards",
        L"bars", L"bears", L"beers", L"boards", L"books",
        L"cars", L"chairs", L"cheers", L"colors", L"doors",
        L"ears", L"errors", L"fears", L"floors", L"gears",
        L"headers", L"hours", L"jazz",
        L"leaders", L"mirrors", L"offers", L"orders",
        L"pairs", L"papers", L"powers", L"prayers",
        L"servers", L"stars", L"tears", L"towers",
        L"users", L"virus", L"wars", L"years",
        // Common single-tone collisions
        L"air", L"also", L"ask", L"box",
        L"bus", L"class", L"cost", L"cross",
        L"data", L"disk", L"docs", L"does", L"door",
        L"down", L"dress", L"express", L"fast",
        L"fix", L"focus", L"for", L"from", L"gas",
        L"glass", L"got", L"has", L"host", L"how",
        L"its", L"just", L"kiss", L"less",
        L"list", L"loss", L"mass", L"miss", L"most",
        L"must", L"next", L"not", L"now",
        L"pass", L"past", L"plus", L"post",
        L"press", L"process", L"rest", L"ross",
        L"six", L"soft", L"stop", L"stress",
        L"success", L"super", L"tax", L"test",
        L"text", L"this", L"thus", L"was", L"yes",
    };
    return words;
}

} // namespace KType
```

**Step 4: Check word list in Commit**

In `src/engine/telex_engine.cpp`, add `#include "wordlist.h"` at the top.

In `Commit()`, add BEFORE the "gi" fixup (after the empty buffer check, around line 416):
```cpp
// English word optimization: reject known English words
if (_config.optimize_multilang >= 1) {
    // Build lowercase version of keyBuffer for lookup
    std::wstring lower = _keyBuffer;
    for (auto& c : lower) c = towlower(c);
    if (EnglishWordList().count(lower)) {
        _result = _keyBuffer;
        _state = TelexStates::CommittedInvalid;
        return _state;
    }
}
```

**Step 5: Build and run tests**

```powershell
& $msbuild tests\tests.vcxproj /p:Configuration=Debug /p:Platform=x64 /nologo /v:minimal
.\tests\build\x64\Debug\tests.exe
```

All tests should pass. The engine tests that don't set config will use the default `optimize_multilang=1`, but the word list only contains English words — no Vietnamese word should match.

**Important:** Verify that no existing Vietnamese test matches the word list. If any do, add exceptions.

**Step 6: Commit**

```bash
git add src/engine/telex.h src/engine/wordlist.h src/engine/telex_engine.cpp tests/test_viettype.cpp
git commit -m "feat: add English word collision detection (optimize_multilang)"
```

---

## Summary

| Task | Priority | Effort | Impact |
|------|----------|--------|--------|
| 1. Space double-press | Critical | Small | Basic usability |
| 2. Uninstall folder | Medium | Tiny | Clean uninstall |
| 3. towupper Vietnamese | Medium | Medium | Correct uppercase |
| 4. English word list | Medium | Medium | Prevents "test"→"tét" |
