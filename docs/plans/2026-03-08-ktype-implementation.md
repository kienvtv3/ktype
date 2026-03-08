# KType Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Build a minimal Vietnamese Telex IME for Windows using TSF (Text Services Framework), C++/ATL/COM.

**Architecture:** 3-layer DLL: Telex Engine (pure C++, portable) → Composition Manager → TSF Integration (ATL/COM). Adapted from VietType patterns with simplified scope (Telex only, no VNI, no macro, no advanced autocorrect).

**Tech Stack:** C++20, ATL/COM, MSBuild, Windows TSF API (msctf.h)

**Reference repos:**
- VietType: `D:/Projects/ktype-master/viettype/` (TSF patterns, Telex engine)
- OpenKey: `D:/Projects/ktype-master/openkey/` (Telex logic, tone placement)

---

## Task 1: Project Scaffold — Solution & Build Files

**Files:**
- Create: `KType.sln`
- Create: `KType.props` (shared build settings)
- Create: `Telex/Telex.vcxproj` (static library)
- Create: `KTypeATL/KTypeATL.vcxproj` (DLL)
- Create: `KTypeATL/KTypeATL.def` (exports)
- Create: `Tests/Tests.vcxproj` (unit tests)

**Step 1: Create solution directory structure**

```
ktype/
├── KType.sln
├── KType.props
├── Telex/
│   └── Telex.vcxproj
├── KTypeATL/
│   └── KTypeATL.vcxproj
└── Tests/
    └── Tests.vcxproj
```

**Step 2: Create KType.props**

Shared build settings (adapted from VietType.props):
- C++20, Unicode, Level 4 warnings
- MultiThreaded runtime (Release), MultiThreadedDebug (Debug)
- Platforms: Win32, x64
- Output: `$(SolutionDir)$(Platform)\$(Configuration)\`
- SDL checks, CFG for Release

**Step 3: Create Telex.vcxproj**

Static library project. No ATL dependency. Include: `TelexEngine.cpp`, headers.

**Step 4: Create KTypeATL.vcxproj**

DLL project with ATL. Links Telex.lib. Precompiled header: `stdafx.h`.
Preprocessor: `KTYPE_EXPORTS`, `_WINDOWS`, `_USRDLL`.

**Step 5: Create KTypeATL.def**

```
LIBRARY
EXPORTS
    DllCanUnloadNow PRIVATE
    DllGetClassObject PRIVATE
    DllRegisterServer PRIVATE
    DllUnregisterServer PRIVATE
```

**Step 6: Create Tests.vcxproj**

Console application linking Telex.lib. For unit testing the engine.

**Step 7: Create KType.sln**

Solution referencing all 3 projects with dependency: KTypeATL → Telex, Tests → Telex.

**Step 8: Verify build**

Run: `msbuild KType.sln /p:Configuration=Debug /p:Platform=x64`
Expected: Build succeeds (empty projects, no source files yet)

**Step 9: Commit**

```bash
git add -A && git commit -m "feat: project scaffold with solution, props, and vcxproj files"
```

---

## Task 2: Telex Engine — Data Tables

**Files:**
- Create: `Telex/Telex.h` (public interface)
- Create: `Telex/TelexData.h` (Vietnamese character data)

**Step 1: Create Telex.h**

Define public interface (adapted from VietType `Telex/Telex.h`):
- `enum class TelexStates { Valid, Invalid, Committed, CommittedInvalid }`
- `enum class Tones { Z, S, F, R, X, J }` (none, sắc, huyền, hỏi, ngã, nặng)
- `struct TelexConfig { bool oa_uy_tone1 = true; }`
- `class ITelexEngine` — abstract interface with:
  - `Reset()`, `PushChar(wchar_t)`, `Backspace()`
  - `Commit()`, `Cancel()`
  - `GetState()`, `Retrieve()`, `RetrieveRaw()`, `Peek()`
  - `Count()`, `AcceptsChar(wchar_t)`

**Step 2: Create TelexData.h**

Adapt from VietType `Telex/TelexData.h`:
- Tone mappings: `transitions_tones` — base vowel → 6 toned variants (a→á/à/ả/ã/ạ)
- Transition maps: `transitions_w` (w-key: o→ơ, u→ư), `transitions_a` (a-key: a→ă)
- Valid C1 (onsets): b, c, ch, d, g, gh, gi, h, k, kh, l, m, n, ng, ngh, nh, p, ph, q, r, s, t, th, tr, v, x, đ
- Valid V (nucleus): a, ai, ao, au, ay, ..., iêu, ươi, ương, etc. with tone position info
- Valid C2 (coda): c, ch, m, n, ng, nh, p, t (with tone restrictions)
- Character classification array for Telex: which chars are vowels, consonants, tones, W, etc.

Only include Telex style (no VNI, no TelexComplicated).

**Step 3: Commit**

```bash
git add Telex/ && git commit -m "feat: Telex engine public interface and Vietnamese data tables"
```

---

## Task 3: Telex Engine — Core State Machine

**Files:**
- Create: `Telex/TelexEngine.h` (class definition)
- Create: `Telex/TelexEngine.cpp` (implementation)

**Step 1: Create TelexEngine.h**

Class definition (adapted from VietType `Telex/TelexEngine.h`):
```cpp
class TelexEngine : public ITelexEngine {
    TelexConfig _config;
    TelexStates _state = TelexStates::Valid;
    std::wstring _keyBuffer;  // raw input
    std::wstring _c1;         // onset consonant
    std::wstring _v;          // vowel nucleus
    std::wstring _c2;         // coda consonant
    Tones _t = Tones::Z;     // current tone
    std::vector<int> _cases;  // case tracking per output char
    // ... helper methods
};
```

**Step 2: Implement PushChar**

Core state machine logic:
1. Classify character (Vowel, ConsoC1, ConsoC2, Tone, W, Dd, etc.)
2. If C1 empty and V empty → try add to C1
3. If vowel → add to V, apply transitions
4. If W → transform vowels (o→ơ, u→ư, a→ă)
5. If tone (s/f/r/x/j/z) → set tone
6. If C2 → add coda consonant
7. Track case (upper/lower) for each char
8. Return Valid or Invalid

**Step 3: Implement Commit**

1. Validate C1 against valid_c1
2. Validate V against valid_v
3. Validate C2 against valid_c2 (check tone restrictions)
4. Find tone position from valid_v table
5. Apply tone to vowel at tone position using transitions_tones
6. Apply case (upper/lower)
7. Return concatenated result: C1 + V(toned) + C2

**Step 4: Implement Backspace**

Remove last character from keyBuffer, replay remaining chars through PushChar.

**Step 5: Implement Retrieve, RetrieveRaw, Peek, Cancel, Reset**

- `Retrieve()`: return committed result string
- `RetrieveRaw()`: return _keyBuffer as-is
- `Peek()`: preview current composition (C1+V+C2 without tone applied)
- `Cancel()`: set state to CommittedInvalid, result = raw keys
- `Reset()`: clear all buffers, state = Valid

**Step 6: Implement AcceptsChar**

Check if character is in Telex character set (a-z, A-Z).

**Step 7: Verify build**

Run: `msbuild Telex/Telex.vcxproj /p:Configuration=Debug /p:Platform=x64`
Expected: Builds successfully

**Step 8: Commit**

```bash
git add Telex/ && git commit -m "feat: Telex engine core state machine"
```

---

## Task 4: Telex Engine — Unit Tests

**Files:**
- Create: `Tests/test_main.cpp`
- Create: `Tests/test_telex.cpp`

**Step 1: Create simple test framework**

Minimal assertion macros (no external dependency):
```cpp
#define ASSERT_EQ(a, b) ...
#define ASSERT_TRUE(x) ...
#define TEST(name) ...
```

**Step 2: Write basic tests**

```cpp
// Simple vowel
TEST(SimpleA) { engine.PushChar('a'); engine.Commit(); ASSERT_EQ(engine.Retrieve(), L"a"); }

// Tone: as → á
TEST(ToneS) { push("as"); engine.Commit(); ASSERT_EQ(engine.Retrieve(), L"á"); }

// Full word: viet → việt (v-i-e-j-t)
TEST(Viet) { push("viejt"); engine.Commit(); ASSERT_EQ(engine.Retrieve(), L"việt"); }

// Full word: nam → nam (no tone)
TEST(Nam) { push("nam"); engine.Commit(); ASSERT_EQ(engine.Retrieve(), L"nam"); }

// Tone placement: hoa → hoà (new style, tone on a)
TEST(HoaTone) { push("hoaf"); engine.Commit(); ASSERT_EQ(engine.Retrieve(), L"hoà"); }

// W transition: uw → ư
TEST(Uw) { push("uw"); engine.Commit(); ASSERT_EQ(engine.Retrieve(), L"ư"); }

// Đ: dd → đ
TEST(Dd) { push("ddi"); engine.Commit(); ASSERT_EQ(engine.Retrieve(), L"đi"); }

// Invalid word: just raw keys
TEST(Invalid) { push("bbb"); engine.Commit(); ASSERT_EQ(engine.Retrieve(), L"bbb"); }

// Backspace
TEST(Backspace) { push("as"); engine.Backspace(); engine.Commit(); ASSERT_EQ(engine.Retrieve(), L"a"); }

// Cancel returns raw
TEST(Cancel) { push("as"); engine.Cancel(); ASSERT_EQ(engine.Retrieve(), L"as"); }
```

**Step 3: Run tests**

Run: `msbuild Tests/Tests.vcxproj /p:Configuration=Debug /p:Platform=x64 && x64\Debug\Tests.exe`
Expected: All tests pass

**Step 4: Commit**

```bash
git add Tests/ && git commit -m "feat: Telex engine unit tests"
```

---

## Task 5: TSF Scaffold — DLL Entry & Globals

**Files:**
- Create: `KTypeATL/stdafx.h` (precompiled header)
- Create: `KTypeATL/stdafx.cpp`
- Create: `KTypeATL/Globals.h`
- Create: `KTypeATL/Globals.cpp`
- Create: `KTypeATL/dllmain.h`
- Create: `KTypeATL/dllmain.cpp`
- Create: `KTypeATL/resource.h`
- Create: `KTypeATL/KTypeATL.rc`

**Step 1: Create stdafx.h**

```cpp
#pragma once
#define STRICT
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#include <atlbase.h>
#include <atlcom.h>
#include <msctf.h>
#include <InputScope.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
```

**Step 2: Create Globals.h/cpp**

Generate new GUIDs for KType (MUST be unique, not copied from VietType):
- `CLSID_TextService` — KType's COM class ID
- `GUID_Profile` — KType's TSF profile GUID
- Constants: `TextServiceDescription = L"KType"`, `ConfigKeyName = L"Software\\KType"`
- `TextServiceLangId = MAKELANGID(LANG_VIETNAMESE, SUBLANG_VIETNAMESE_VIETNAM)`
- `extern HINSTANCE DllInstance`

**Step 3: Create dllmain.h/cpp**

ATL module class + DllMain storing DllInstance (adapted from VietType pattern).

**Step 4: Create resource.h and KTypeATL.rc**

Minimal resources: one icon (IDI_KTYPE), string table with description.

**Step 5: Verify build**

Run: `msbuild KTypeATL/KTypeATL.vcxproj /p:Configuration=Debug /p:Platform=x64`
Expected: DLL builds (no TSF functionality yet)

**Step 6: Commit**

```bash
git add KTypeATL/ && git commit -m "feat: TSF scaffold - DLL entry, globals, resources"
```

---

## Task 6: TSF TextService — Activation & Registration

**Files:**
- Create: `KTypeATL/TextService.h`
- Create: `KTypeATL/TextService.cpp`
- Create: `KTypeATL/Register.cpp`
- Create: `KTypeATL/KTypeATL.cpp` (COM exports)

**Step 1: Create TextService class**

Implement (adapted from VietType `TextService.h/cpp`):
- `ITfTextInputProcessorEx` — `ActivateEx()`, `Deactivate()`
- `ITfDisplayAttributeProvider` — `EnumDisplayAttributeInfo()`, `GetDisplayAttributeInfo()`
- ATL COM map with `OBJECT_ENTRY_AUTO`
- On ActivateEx: store ITfThreadMgr, create ContextManager
- On Deactivate: cleanup

**Step 2: Create Register.cpp**

- `DllRegisterServer()` → `RegisterProfiles()` + `RegisterCategories()`
- `DllUnregisterServer()` → unregister
- Register categories: TIP_KEYBOARD, UIELEMENTENABLED, IMMERSIVESUPPORT, SYSTRAYSUPPORT, DISPLAYATTRIBUTEPROVIDER
- **Key difference from VietType**: ensure clean profile registration to avoid the "3rd keyboard layout" bug

**Step 3: Create KTypeATL.cpp**

Standard COM DLL exports forwarding to ATL module.

**Step 4: Verify build**

Run: `msbuild KTypeATL/KTypeATL.vcxproj /p:Configuration=Debug /p:Platform=x64`
Expected: DLL builds with exported functions

**Step 5: Commit**

```bash
git add KTypeATL/ && git commit -m "feat: TextService activation and TSF registration"
```

---

## Task 7: TSF Key Event Handling

**Files:**
- Create: `KTypeATL/ContextManager.h`
- Create: `KTypeATL/ContextManager.cpp`
- Create: `KTypeATL/KeyEventSink.cpp`
- Create: `KTypeATL/KeyTranslator.h`
- Create: `KTypeATL/KeyTranslator.cpp`

**Step 1: Create ContextManager**

Implements (adapted from VietType `ContextManager`):
- `ITfThreadMgrEventSink` — `OnInitDocumentMgr()`, `OnSetFocus()`, etc.
- `ITfKeyEventSink` — `OnKeyDown()`, `OnKeyUp()`, `OnTestKeyDown()`, `OnTestKeyUp()`
- Manages map of `ITfContext → Context` objects
- Advises key event sink and thread mgr event sink on init

**Step 2: Create KeyTranslator**

Translate VK codes to wchar_t using `ToUnicode()` API.
- `IsKeyEaten()` — should this key be processed by KType?
- `IsEditKey()` — is this Enter/Tab/Esc/arrows?
- Logic: eat alphabetic keys (a-z) when composing, pass through modifiers (Ctrl/Alt)

**Step 3: Create KeyEventSink.cpp**

`OnKeyDown` flow:
1. Translate key via KeyTranslator
2. If edit key (Enter/Tab/Esc) → commit current composition, pass key through
3. If non-alphabetic (space, punctuation, `_`) → commit composition, pass key through
4. If alphabetic → feed to Context for Telex processing

**Step 4: Verify build**

Run: `msbuild KType.sln /p:Configuration=Debug /p:Platform=x64`
Expected: Full solution builds

**Step 5: Commit**

```bash
git add KTypeATL/ && git commit -m "feat: key event handling and translation"
```

---

## Task 8: TSF Composition Management

**Files:**
- Create: `KTypeATL/Context.h`
- Create: `KTypeATL/Context.cpp`
- Create: `KTypeATL/EditSession.h`
- Create: `KTypeATL/EditSessions.cpp`
- Create: `KTypeATL/DisplayAttributes.h`
- Create: `KTypeATL/DisplayAttributes.cpp`

**Step 1: Create EditSession template**

Generic edit session wrapper (adapted from VietType `EditSession.h`):
```cpp
template <typename... Args>
class EditSession : public ITfEditSession {
    // Stores callback + args, invokes via DoEditSession
};
```

**Step 2: Create Context class**

Per-input-context composition handler:
- Holds `ITfContext`, `ITfComposition`, `TelexEngine` instance
- `ITfCompositionSink` — `OnCompositionTerminated()`
- Key methods:
  - `RequestEditKey()` — request async edit session
  - `EditKey()` — process key within edit session
  - `EditNextState()` — update composition text after engine state change
  - `EditCommit()` — finalize composition (commit engine, end composition)
  - `EnsureCompositionText()` — start composition if not started
  - `SetCompositionText()` — update displayed text
  - `EndCompositionNow()` — end composition, insert final text

**Critical behavior:**
- Enter/Tab/Esc/focus loss → always call `EditCommit()` before passing key
- `_` and punctuation → `EditCommit()` then pass key through
- This fixes the VietType "folder rename" bug and Windows Telex underscore bug

**Step 3: Create DisplayAttributes**

Simple dotted underline for active composition (adapted from VietType).

**Step 4: Verify build**

Run: `msbuild KType.sln /p:Configuration=Debug /p:Platform=x64`
Expected: Full solution builds

**Step 5: Commit**

```bash
git add KTypeATL/ && git commit -m "feat: composition management and edit sessions"
```

---

## Task 9: Integration — Wire Everything Together

**Files:**
- Modify: `KTypeATL/TextService.cpp` — create ContextManager on activate
- Modify: `KTypeATL/ContextManager.cpp` — create Context on focus, route keys
- Modify: `KTypeATL/Context.cpp` — integrate TelexEngine

**Step 1: Wire TextService → ContextManager**

In `ActivateEx()`:
1. Store ITfThreadMgr
2. Create ContextManager
3. ContextManager advises key event sink + thread mgr event sink
4. **Set default_enabled = true** (Vietnamese input active immediately on switch)

In `Deactivate()`:
1. ContextManager unadvises all sinks
2. Cleanup

**Step 2: Wire ContextManager → Context**

In `OnInitDocumentMgr()` / `OnSetFocus()`:
1. Get ITfContext from document
2. Create Context with new TelexEngine instance
3. Store in context map

**Step 3: Wire Context → TelexEngine**

In `EditKey()`:
1. Call `_engine->PushChar(wchar_t)` or `_engine->Backspace()`
2. Check state: Valid → update composition text via `Peek()`
3. If commit trigger → `_engine->Commit()`, get `Retrieve()`, end composition

**Step 4: Handle composition commit triggers**

Ensure these all commit and clear:
- Space, punctuation (`.`, `,`, `;`, `:`, `!`, `?`, etc.)
- Underscore `_`
- Enter, Tab, Escape
- Focus loss (`OnEndEdit`, `OnCompositionTerminated`)
- Non-alphabetic keys

**Step 5: Verify build**

Run: `msbuild KType.sln /p:Configuration=Debug /p:Platform=x64`
Expected: Complete DLL builds

**Step 6: Commit**

```bash
git add KTypeATL/ && git commit -m "feat: wire all components together"
```

---

## Task 10: Manual Testing & Registration

**Step 1: Register DLL**

Run (admin): `regsvr32 x64\Debug\KTypeATL.dll`
Expected: "DllRegisterServer succeeded"

**Step 2: Verify in Windows Settings**

Open Settings → Time & Language → Language → Vietnamese → KType should appear as keyboard option.

**Step 3: Test basic input**

1. Open Notepad
2. Switch to KType via Win+Space
3. Type `xin chaof` → should see `xin chào`
4. Type `viejt nam` → should see `việt nam`
5. Type `snake_case` → should see `snake_case` (not duplicated)
6. Rename a folder → Enter should commit text

**Step 4: Fix issues found during manual testing**

Iterate on bugs discovered.

**Step 5: Commit fixes**

```bash
git commit -am "fix: issues found during manual testing"
```

---

## Task 11: Settings via Registry

**Files:**
- Create: `KTypeATL/EngineSettings.h`
- Create: `KTypeATL/EngineSettings.cpp`

**Step 1: Implement registry settings reader**

Read from `HKEY_CURRENT_USER\Software\KType`:
- `oa_uy_tone1` (DWORD, default 1) — tone placement style

**Step 2: Load settings on ContextManager init**

Apply TelexConfig from registry when creating TelexEngine instances.

**Step 3: Commit**

```bash
git add KTypeATL/ && git commit -m "feat: registry-based settings"
```

---

## Task 12: Input Scope Detection

**Files:**
- Modify: `KTypeATL/Context.cpp` — detect password/restricted fields

**Step 1: Check input scope on context creation**

Query `ITfContext` for `GUID_PROP_INPUTSCOPE`. If scope is:
- `IS_PASSWORD`, `IS_PIN` → disable KType for this context
- This auto-disables Vietnamese input in password fields

**Step 2: Commit**

```bash
git add KTypeATL/ && git commit -m "feat: auto-disable in password fields"
```

---

## Build & Test Commands Reference

```bash
# Build all (Debug, x64)
msbuild KType.sln /p:Configuration=Debug /p:Platform=x64

# Build all (Release, x64)
msbuild KType.sln /p:Configuration=Release /p:Platform=x64

# Run unit tests
x64\Debug\Tests.exe

# Register (admin)
regsvr32 x64\Debug\KTypeATL.dll

# Unregister (admin)
regsvr32 /u x64\Debug\KTypeATL.dll
```
