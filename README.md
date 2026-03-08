# KType

A Vietnamese input method editor (IME) for Windows using the Text Services Framework (TSF).

## What is KType?

KType is a native Windows IME that lets you type Vietnamese using the Telex input method. Unlike traditional Vietnamese input tools (Unikey, EVKey) that simulate backspace keystrokes to replace text, KType uses Windows TSF — the same framework used by Microsoft's own IMEs — to compose text directly in applications.

### Why TSF?

| Feature | Fake Backspace (Unikey/EVKey) | TSF (KType) |
|---|---|---|
| Text composition | Simulates backspace + retype | Direct in-place editing |
| Application compatibility | Breaks in some apps (games, terminals) | Native integration |
| Composition preview | Limited or none | Underlined text while typing |
| Security software | Can be flagged as keylogger | Standard Windows API |

## Features

### Input Method
- **Telex input**: Standard Telex key mappings (aa→â, aw→ă, ow→ơ, dd→đ, etc.)
- **Tone marks**: s(sắc), f(huyền), r(hỏi), x(ngã), j(nặng), z(remove tone)
- **Tone toggle**: Pressing the same tone key twice removes the tone
- **Tone replacement**: Pressing a different tone key replaces the current tone

### Vietnamese Orthography
- **Syllable validation**: Validates C1 (onset) + V (vowel) + C2 (coda) structure
- **Tone placement**: Follows Vietnamese orthographic rules for diacritic positioning
- **New/old style**: Configurable tone placement — "hoà" (new) vs "hòa" (old) for oa/oe/uy vowel pairs
- **Restricted codas**: Automatically enforces that stop consonants (c, ch, k, p, t) only accept sắc or nặng tones
- **Consonant clusters**: Full support for digraphs/trigraphs (ch, gh, gi, kh, ng, ngh, nh, ph, qu, th, tr)

### Engine
- **Real-time composition**: Character-by-character processing with instant feedback
- **Backspace support**: Removes the last input character and replays the sequence
- **Peek/preview**: Shows the current composition state without committing
- **Case preservation**: Maintains uppercase/lowercase from the original keystrokes

## Building

Requires Visual Studio Build Tools 2026 (or later) with:
- C++ Desktop Development workload
- ATL (Active Template Library)

```powershell
$msbuild = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\MSBuild.exe"

# Build everything
& $msbuild KType.sln /p:Configuration=Release /p:Platform=x64

# Build and run tests
& $msbuild tests\tests.vcxproj /p:Configuration=Debug /p:Platform=x64
.\tests\build\x64\Debug\tests.exe
```

## Installation

```powershell
# Register the IME (requires administrator)
regsvr32 KType.dll

# Unregister
regsvr32 /u KType.dll
```

After registration, KType appears in Windows Settings > Time & Language > Language > Keyboard as a Vietnamese input method.

## Project Structure

```
src/engine/    # Core Telex engine — pure C++, no Windows dependencies
src/tsf/       # Windows TSF integration — COM/ATL DLL
tests/         # Test suite — 104 tests across 7 categories
```

## License

GPL-3.0 — see [LICENSE](LICENSE) for details.
