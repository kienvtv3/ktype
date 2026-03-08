# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**KType** is a Vietnamese input method editor (IME) for Windows, built on the TSF (Text Services Framework) API. Unlike EVKey or Unikey which use a "fake backspace" approach (simulating backspace keys to replace text), KType uses the TSF composition model similar to VietType — directly composing text in the application's edit control without keystroke simulation.

## Architecture

### TSF-based IME Model
- Uses Windows TSF (Text Services Framework) COM interfaces
- Implements `ITfTextInputProcessor` as the main text service entry point
- Uses composition objects (`ITfCompositionSink`, `ITfComposition`) to manage in-progress text
- Registers as a COM server (DLL) that Windows loads into each application process

### Key TSF Concepts
- **Text Service**: The DLL registered as a COM class implementing TSF interfaces
- **Composition**: A range of text being actively edited by the IME before finalization
- **Edit Session**: Thread-safe mechanism to modify document text via `ITfEditSession`
- **Key Event Sink**: Intercepts keystrokes via `ITfKeyEventSink` before they reach the app
- **Language Bar**: Provides UI integration in the system tray/language bar

### Vietnamese Input Processing
- Supports Telex, VNI, and potentially other input methods
- Handles tone mark placement according to Vietnamese orthographic rules
- Processes diacritical marks (dấu) composition in real-time

## Build

This is a Windows C/C++ project. Build using Visual Studio or MSBuild:

```
msbuild ktype.sln /p:Configuration=Release /p:Platform=x64
```

### Registration
TSF IMEs must be registered as COM servers:
```
regsvr32 ktype.dll        # Register
regsvr32 /u ktype.dll     # Unregister
```

## Key Technical Constraints

- Must be compiled as a DLL (loaded into every process that uses text input)
- Must be thread-safe — TSF calls can come from any thread
- COM reference counting (`AddRef`/`Release`) must be correctly implemented to avoid leaks
- Must handle both 32-bit and 64-bit builds (Windows loads the matching architecture)
- Minimize memory footprint since the DLL is loaded into every process
- Cannot use fake backspace approach — all text manipulation goes through TSF composition

## Repository

- Remote: https://github.com/kienvtv3/ktype.git
