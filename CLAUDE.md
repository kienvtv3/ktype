# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**KType** is a Vietnamese input method editor (IME) for Windows, built on the TSF (Text Services Framework) API. Unlike EVKey or Unikey which use a "fake backspace" approach (simulating backspace keys to replace text), KType uses the TSF composition model similar to VietType — directly composing text in the application's edit control without keystroke simulation.

## Project Structure (Pitchfork Layout)

```
ktype/
├── src/
│   ├── engine/          # Core Telex engine (static library)
│   │   ├── telex.h          # Public API: TelexEngine, TelexStates, TelexConfig
│   │   ├── telex_data.h     # Vietnamese character tables, tone data, consonant/vowel sets
│   │   └── telex_engine.cpp # Engine state machine implementation
│   └── tsf/             # Windows TSF integration (DLL)
│       ├── text_service.cpp # ITfTextInputProcessor implementation
│       ├── context_manager.cpp/h # ITfKeyEventSink, composition management
│       ├── edit_session.cpp/h   # ITfEditSession for thread-safe text edits
│       ├── register.cpp         # COM/TSF registration (DllRegisterServer)
│       ├── display_attribute.cpp/h # Underline styling for composition text
│       ├── compartment.h        # TSF compartment helpers
│       ├── globals.cpp/h        # GUIDs, constants
│       └── pch.h                # Precompiled header (ATL, TSF, Windows)
├── tests/               # Test suite (console application)
│   ├── test_helper.h        # Shared macros (ASSERT_WSTR_EQ, RUN_TEST, commit helper)
│   ├── main.cpp             # Test runner entry point
│   ├── test_tones.cpp       # 23 tests: tone marks, toggle, replace, placement
│   ├── test_vowels.cpp      # 22 tests: circumflex, W transitions, di-vowels, undo
│   ├── test_consonants.cpp  # 28 tests: dd→đ, C1/C2 clusters, restricted codas
│   ├── test_words.cpp       # 40 tests: common Vietnamese words end-to-end
│   ├── test_edge_cases.cpp  # 18 tests: tone position styles, peek, cancel, gi-
│   ├── test_backspace.cpp   # 8 tests: backspace replay behavior
│   ├── test_case.cpp        # 6 tests: uppercase handling
│   └── test_viettype.cpp    # 119 tests: VietType compat (ported from TestTelex.cpp)
├── KType.sln            # Visual Studio solution (Engine, TSF, Tests projects)
└── KType.props          # Shared MSBuild properties (C++20, W4, Unicode)
```

## Naming Conventions (Google C++ Style Guide)

- **Files**: `snake_case.cpp`, `snake_case.h`
- **Classes/Structs**: `PascalCase` (e.g., `TelexEngine`, `TextService`)
- **Methods**: `PascalCase` (e.g., `PushChar`, `TryAddVowel`)
- **Member variables**: `_camelCase` prefix (e.g., `_keyBuffer`, `_state`)
- **Namespaces**: `PascalCase` (`KType`, `TelexData`)

## Build

Requires Visual Studio Build Tools 2026 (v18) with C++ Desktop workload and ATL.

```powershell
# Build via PowerShell (MSBuild path)
$msbuild = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\MSBuild.exe"

# Build entire solution
& $msbuild KType.sln /p:Configuration=Debug /p:Platform=x64 /nologo /v:minimal

# Build and run tests only
& $msbuild tests\tests.vcxproj /p:Configuration=Debug /p:Platform=x64 /nologo /v:minimal
.\tests\build\x64\Debug\tests.exe
```

Output: `build\x64\{Debug|Release}\`

### Registration
TSF IMEs must be registered as COM servers:
```
regsvr32 KType.dll        # Register
regsvr32 /u KType.dll     # Unregister
```

## Architecture

### Engine (Static Library)
The Telex engine is a pure C++ state machine with no Windows dependencies. It processes characters one at a time, building a Vietnamese syllable from components:
- **C1** (onset consonant): b, ch, gi, ng, ngh, ph, qu, tr, đ, etc.
- **V** (vowel nucleus): single, double, or triple vowels with diacritics
- **C2** (coda consonant): c, ch, m, n, ng, nh, p, t
- **Tone**: sắc(S), huyền(F), hỏi(R), ngã(X), nặng(J), none(Z)

Key behaviors:
- `PushChar()` → feeds characters, returns Valid/Invalid state
- `Commit()` → validates syllable structure and produces output
- `Backspace()` → removes last char and replays from scratch
- `Peek()` → preview current composition without committing
- Restricted C2 (c, ch, k, p, t) only allow sắc or nặng tones
- `oa_uy_tone1` config: new style "hoà" (default) vs old style "hòa"

### TSF (DLL)
COM/ATL-based Windows integration:
- `TextService` → main entry point, implements `ITfTextInputProcessorEx`
- `ContextManager` → key event sink, manages composition lifecycle
- `EditSession` → thread-safe document text manipulation
- Uses composition model (not fake backspace)

## Key Technical Constraints

- DLL loaded into every process — minimize memory footprint
- Thread-safe: TSF calls can come from any thread
- COM reference counting must be correct to avoid leaks
- `towupper()` doesn't handle Vietnamese diacritics on Windows (known limitation)
- Must handle both x86 and x64 (Windows loads matching architecture)

## Repository

- Remote: https://github.com/kienvtv3/ktype.git
- License: GPL-3.0
