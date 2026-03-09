# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**KType** is a Vietnamese input method editor (IME) for Windows, built on TSF (Text Services Framework). Unlike EVKey or Unikey which simulate backspace keystrokes, KType uses the TSF composition model — directly composing text in the application's edit control without keystroke simulation. This makes it work correctly in terminals (Claude Code, Windows Terminal, VS Code).

## Project Structure (Pitchfork Layout)

```
ktype/
├── src/
│   ├── engine/              # Core Telex engine (static library, no Windows deps)
│   │   ├── telex.h              # Public API: TelexEngine, TelexStates, TelexConfig
│   │   ├── telex_data.h         # Vietnamese character tables, tone data, vowel/consonant sets
│   │   └── telex_engine.cpp     # Engine state machine implementation
│   └── tsf/                 # Windows TSF integration (COM/ATL DLL)
│       ├── text_service.cpp/h   # ITfTextInputProcessorEx entry point
│       ├── context_manager.cpp/h # ITfKeyEventSink, composition lifecycle
│       ├── context.cpp/h        # ITfContext wrapper, composition state
│       ├── key_handler.cpp      # Key event processing (OnKeyDown/OnTestKeyDown)
│       ├── key_translator.cpp/h # Virtual key → character translation
│       ├── edit_session.h       # ITfEditSession for thread-safe text edits
│       ├── register.cpp         # COM/TSF registration, InstallTip, ACL setup
│       ├── display_attribute.cpp/h # Underline styling for composition text
│       ├── compartment.cpp/h    # TSF compartment helpers
│       ├── globals.cpp/h        # GUIDs, constants
│       ├── dll_main.cpp/h       # DLL entry point, COM class factory
│       ├── exports.cpp          # DllRegisterServer/DllUnregisterServer
│       └── pch.h                # Precompiled header (ATL, TSF, Windows, ACL)
├── tests/                   # Test suite — 264 tests (console application)
│   ├── test_helper.h            # Shared macros (ASSERT_WSTR_EQ, RUN_TEST, commit helper)
│   ├── main.cpp                 # Test runner entry point
│   ├── test_tones.cpp           # 23 tests: tone marks, toggle, replace, placement
│   ├── test_vowels.cpp          # 22 tests: circumflex, W transitions, di-vowels, undo
│   ├── test_consonants.cpp      # 28 tests: dd→đ, C1/C2 clusters, restricted codas
│   ├── test_words.cpp           # 40 tests: common Vietnamese words end-to-end
│   ├── test_edge_cases.cpp      # 18 tests: tone position styles, peek, cancel, gi-
│   ├── test_backspace.cpp       # 8 tests: backspace replay behavior
│   ├── test_case.cpp            # 6 tests: uppercase handling
│   └── test_viettype.cpp        # 119 tests: VietType compat (ported from TestTelex.cpp)
├── installer/               # Inno Setup installer
│   ├── ktype.iss                # Installer script (regsvr32, keyboard setup)
│   ├── setup-keyboard.ps1      # Post-install: add Vietnamese language, set KType as IME
│   ├── cleanup-keyboard.ps1    # Uninstall: remove KType, restore Vietnamese Telex
│   └── ktype.ico               # Installer icon
├── docs/                    # Documentation
│   ├── viettype-reference.md    # VietType engine analysis (reference material)
│   └── plans/                   # Historical design/implementation plans
├── release.ps1              # Build + test + installer + GitHub release script
├── KType.sln                # Visual Studio solution (Engine, TSF, Tests projects)
└── KType.props              # Shared MSBuild properties (C++20, W4, Unicode)
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
$msbuild = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\MSBuild.exe"

# Build entire solution
& $msbuild KType.sln /p:Configuration=Debug /p:Platform=x64 /nologo /v:minimal

# Build and run tests only
& $msbuild tests\tests.vcxproj /p:Configuration=Debug /p:Platform=x64 /nologo /v:minimal
.\tests\build\x64\Debug\tests.exe
```

Output: `build\x64\{Debug|Release}\`

### Release

```powershell
.\release.ps1                    # Auto-increment patch version
.\release.ps1 -Version v0.3.0   # Specific version
.\release.ps1 -Overwrite        # Overwrite existing release
```

The release script: builds Release → runs 264 tests → builds Inno Setup installer → publishes to GitHub Releases.

## Architecture

### Engine (Static Library)
Pure C++ state machine with no Windows dependencies. Processes characters one at a time, building a Vietnamese syllable:
- **C1** (onset): b, ch, gi, kh, ng, ngh, nh, ph, qu, th, tr, đ
- **V** (vowel): single/double/triple vowels with diacritics (iê, uô, ươ, oa, oe, uy, ...)
- **C2** (coda): c, ch, k, m, n, ng, nh, p, t
- **Tone**: sắc(S), huyền(F), hỏi(R), ngã(X), nặng(J), none(Z)

Key behaviors:
- `PushChar()` → feeds characters, returns Valid/Invalid state. Permissive acceptance — validation deferred to Commit
- `Commit()` → validates syllable structure (C2Mode, tone restrictions) and produces output
- `Backspace()` → removes last char and replays entire sequence from scratch
- `Peek()` → preview current composition without committing
- `CheckInvariants()` → debug assertions (5 checks) for state consistency
- TryAddVowel uses O(1) map-based lookups (VowelTransitionMap, ValidVowelPrefixSet)
- Restricted C2 (c, ch, k, p, t) only allow sắc or nặng tones
- `oa_uy_tone1` config: new style "hoà" (default) vs old style "hòa"

### TSF (DLL)
COM/ATL-based Windows integration:
- `TextService` → main entry point, implements `ITfTextInputProcessorEx`
- `ContextManager` → key event sink, manages composition lifecycle
- `EditSession` → thread-safe document text manipulation
- `register.cpp` → COM registration, COMLESS category, InstallTip with SetDefaultLayoutOrTip, UWP Settings ACL, dynamic category unregistration
- Uses composition model (not fake backspace)

### Installer
Inno Setup-based:
- Registers DLL via regsvr32 (handles locked DLL with restartreplace)
- Runs setup-keyboard.ps1 to add Vietnamese language and set KType as default IME
- Uninstall runs cleanup-keyboard.ps1 to restore Vietnamese Telex

## Key Technical Constraints

- DLL loaded into every process — minimize memory footprint
- Thread-safe: TSF calls can come from any thread
- COM reference counting must be correct to avoid leaks
- `towupper()` doesn't handle Vietnamese diacritics on Windows (known limitation)
- Use PowerShell to call MSBuild from scripts (bash strips `/` from MSBuild switches)

## Repository

- Remote: https://github.com/kienvtv3/ktype.git
- License: GPL-3.0
