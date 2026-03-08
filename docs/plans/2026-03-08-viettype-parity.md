# VietType Telex Parity Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Make KType's Telex engine behave identically to VietType's default Telex mode (no VNI, no TelexComplicated, no abbreviations).

**Architecture:** Extend KType's existing engine with missing VietType behaviors: C2Mode validation, missing transitions, accept_separate_dd, xoong handling, transitions_wv_c2 (vowel adjustment on C2), and autocorrect. All changes stay within `src/engine/` (static lib) and `tests/`.

**Tech Stack:** C++20, MSBuild, no external dependencies.

**Reference:** VietType source at `D:\Projects\ktype-master\VietType\Telex\`, analysis in `memory/viettype-engine.md`.

---

## Gap Summary (KType vs VietType default Telex)

| # | Feature | VietType | KType | Priority |
|---|---------|----------|-------|----------|
| 1 | C2Mode validation (MustC2/NoC2) | Commit enforces | Fields exist but not enforced | High |
| 2 | Missing vowel transitions (ô+o→oo reverse, ư+o→ươ relaxed) | Yes | No | High |
| 3 | accept_separate_dd (dd after vowels) | Config, default true | Only before vowels | High |
| 4 | transitions_wv_c2 (vowel adjust on C2) | uơ→ươ, ưa→uă, ưo→ươ | No | High |
| 5 | Max word length (10 chars) | Enforced | No limit | Medium |
| 6 | Tone replacement behavior | Replaces tone (same tone = undo) | Same | OK |
| 7 | xoong special case (oo after C2 invalidates) | Line 328 | Transition fires after C2 | Medium |
| 8 | Autocorrect (w-reorder, ie→iê, trailing h/g) | Optional, default OFF | No | Low (default off) |
| 9 | English dictionary optimization | Level 1 default | No | Low |
| 10 | Backconvert (Vietnamese→Telex keys) | Yes | No | Future (TSF needs) |

**Scope:** Tasks 1-7 (default-on behaviors). Tasks 8-10 are deferred (default-off or TSF-only).

---

### Task 1: Enforce C2Mode (requiresC2 / forbidsC2) in Commit

VietType rejects invalid syllables at Commit: "iê" without C2 → invalid, "ai" + C2 → invalid.

**Files:**
- Modify: `src/engine/telex_engine.cpp` (Commit function, ~line 400)
- Modify: `tests/test_edge_cases.cpp` (add C2Mode tests)

**Step 1: Write failing tests**

Add to `tests/test_edge_cases.cpp`:
```cpp
// C2Mode: MustC2 — iê requires coda
bool test_ie_no_c2()   { ASSERT_WSTR_EQ(commit("iee"), L"iee"); return true; }          // iê alone → invalid
bool test_ie_with_c2() { ASSERT_WSTR_EQ(commit("ieeng"), L"i\x1ebfng"); return true; }  // iêng → valid

// C2Mode: NoC2 — ai forbids coda
bool test_ai_no_c2()   { ASSERT_WSTR_EQ(commit("ai"), L"ai"); return true; }             // ai → valid
bool test_ai_with_c2() { ASSERT_WSTR_EQ(commit("ain"), L"ain"); return true; }           // ain → invalid (raw)

// C2Mode: uô requires C2
bool test_uo_no_c2()   { ASSERT_WSTR_EQ(commit("uoo"), L"uoo"); return true; }          // uô alone → invalid
bool test_uo_with_c2() { ASSERT_WSTR_EQ(commit("uoong"), L"u\x1ed3ng"); return true; }  // uông → valid
```

Register them in `run_edge_case_tests()`.

**Step 2: Run tests, verify they fail**

```powershell
& $msbuild tests\tests.vcxproj /p:Configuration=Debug /p:Platform=x64 /nologo /v:minimal
.\tests\build\x64\Debug\tests.exe
```

Expected: new tests FAIL (MustC2 words currently pass without C2, NoC2 words pass with C2).

**Step 3: Implement C2Mode validation in Commit()**

In `telex_engine.cpp`, after the existing vowel validation block (~line 400-404), add:

```cpp
// Validate C2Mode constraints
if (!_v.empty() && VowelMap().count(_v)) {
    const auto& vi = VowelMap().at(_v);
    if (vi.requiresC2 && _c2.empty()) {
        _result = _keyBuffer;
        _state = TelexStates::CommittedInvalid;
        return _state;
    }
    if (vi.forbidsC2 && !_c2.empty()) {
        _result = _keyBuffer;
        _state = TelexStates::CommittedInvalid;
        return _state;
    }
}
```

**Step 4: Run tests, verify all pass**

**Step 5: Commit**

```
feat: enforce C2Mode (requiresC2/forbidsC2) in Commit validation
```

---

### Task 2: Add Missing Vowel Transitions

VietType has transitions KType lacks:
- `ô+o → oo` (reverse circumflex, enables "xoong" via "xoongo" or natural re-typing)
- `ư+o → ươ` (relaxed horn after W already applied)

**Files:**
- Modify: `src/engine/telex_data.h` (VowelTransitions table)
- Modify: `tests/test_words.cpp` or `tests/test_vowels.cpp`

**Step 1: Write failing tests**

```cpp
// Reverse circumflex: typing o after ô undoes it (for "xoong")
bool test_xoong()  { ASSERT_WSTR_EQ(commit("xoong"), L"xoong"); return true; }  // x+oo→ô, +n(C2), +g(C2 cont)... hmm

// Relaxed ư+o→ươ
bool test_uw_o()   { ASSERT_WSTR_EQ(commit("tuwon"), L"t\x01b0\x01a1n"); return true; }  // tươn
```

Note: "xoong" is tricky — need to think through the exact keystroke sequence. In VietType: x+o+o→xô, then o→ô+o→oo (reverse transition), then n+g → xoong. This requires the reverse transition in the table.

**Step 2: Add transitions to VowelTransitions[]**

```cpp
// Reverse: ô+o → oo (undo circumflex, for "xoong")
{ L"\x00f4o", L"oo" },

// Relaxed: ư+o → ươ (horn already applied via W, then type 'o')
{ L"\x01b0o", L"\x01b0\x01a1" },
```

**Step 3: Run tests, verify pass**

**Step 4: Handle "xoong" edge case**

VietType line 328: if C2 is non-empty and _v=="oo" after appending → invalidate. This prevents the "oo" transition from firing AFTER C2. Need to check if KType's current after-C2 transition logic handles this correctly — the reverse `ô+o→oo` should still fire (it makes _v longer, not shorter), but we need the "oo" to NOT trigger `oo→ô` again.

Actually the flow is: x+o(V="o")+o(V="oo"→ô)+n(C2)+g(C2="ng"). So "xoong" = x+o+o+n+g. After oo→ô, we get xô+ng = "xông". That's wrong for "xoong" (the pot).

VietType fixes this with the reverse transition: after ô is formed, typing another 'o' triggers ô+o→oo (reverse). So "xooong" would give "xoong". But typing exactly "xoong" gives "xông".

KType currently: x+o+o→ô (transition), +n(C2), +g(C2="ng") → "xông". Same as VietType. The word "xoong" requires "xooong" (3 o's). This matches VietType behavior.

**Step 5: Commit**

```
feat: add reverse ô+o→oo and relaxed ư+o→ươ transitions
```

---

### Task 3: accept_separate_dd (dd after vowels)

VietType default: `accept_separate_dd = true` — typing 'd' twice produces đ even after vowels are entered. Example: "add" → "ađ", or editing flow where user types vowels first then goes back.

**Files:**
- Modify: `src/engine/telex_engine.cpp` (TryAddD, PushChar)
- Modify: `src/engine/telex.h` (TelexConfig)
- Modify: `tests/test_consonants.cpp`

**Step 1: Write failing test**

```cpp
// accept_separate_dd: dd works after vowels
bool test_dd_after_vowel() { ASSERT_WSTR_EQ(commit("oadd"), L"oa\x0111"); return true; }  // oa+dd → oađ
```

**Step 2: Implement**

In `telex.h`, add to TelexConfig:
```cpp
bool accept_separate_dd = true;
```

In `telex_engine.cpp`, modify TryAddD:
```cpp
bool TelexEngine::TryAddD(wchar_t /*c*/) {
    if (_c1.empty() && _v.empty() && _c2.empty()) {
        _c1 = L"d";
        return true;
    }
    if (_c1 == L"d" && !_hasD) {
        if (_config.accept_separate_dd || (_v.empty() && _c2.empty())) {
            _c1 = L"\x0111";
            _hasD = true;
            return true;
        }
    }
    return false;
}
```

Wait — this doesn't handle dd AFTER vowels. If vowels exist, PushChar classifies 'd' as Consonant (not DoubleD). Need to also check: if _c1=="d" and we already have vowels, the second 'd' should still be caught.

Actually looking at KType's Classify(): `if (lc == L'd') return CharType::DoubleD;` — 'd' is ALWAYS classified as DoubleD regardless of state. So PushChar always routes to TryAddD. The issue is just in TryAddD's guard: `if (_v.empty() && _c2.empty())`.

Fix TryAddD to remove the _v/_c2 guard when accept_separate_dd is true:

```cpp
bool TelexEngine::TryAddD(wchar_t /*c*/) {
    // First 'd': set as C1 (only if nothing typed yet)
    if (_c1.empty() && _v.empty() && _c2.empty()) {
        _c1 = L"d";
        return true;
    }
    // Second 'd': convert to đ
    if (_c1 == L"d" && !_hasD) {
        if (_config.accept_separate_dd || (_v.empty() && _c2.empty())) {
            _c1 = L"\x0111";
            _hasD = true;
            return true;
        }
    }
    // Third 'd' after đ: undo (invalidate)
    // Already handled by returning false → goes invalid
    return false;
}
```

**Step 3: Run tests, verify pass**

**Step 4: Commit**

```
feat: add accept_separate_dd config (dd works after vowels)
```

---

### Task 4: Vowel Adjustment on C2 (transitions_wv_c2)

When a C2 consonant is typed, VietType adjusts certain vowels:
- `uơ → ươ` (after "quow" + C2)
- `ưa → uă` (after "uwa" + C2, changes horn to breve)
- `ưo → ươ` (after W + C2)

**Files:**
- Modify: `src/engine/telex_data.h` (add WvC2Transitions table)
- Modify: `src/engine/telex_engine.cpp` (TryAddC2)
- Modify: `tests/test_words.cpp`

**Step 1: Write failing tests**

```cpp
// Vowel adjusts when C2 typed
bool test_uwat()  { ASSERT_WSTR_EQ(commit("uwat"), L"u\x0103t"); return true; }  // ưa+t → uăt (horn→breve)
bool test_uwon()  { ASSERT_WSTR_EQ(commit("uwon"), L"\x01b0\x01a1n"); return true; }  // ưo+n → ươn
```

**Step 2: Add data table**

In `telex_data.h`:
```cpp
// Vowel adjustments triggered when C2 is typed (VietType: transitions_wv_c2)
inline const WTransition WvC2Transitions[] = {
    { L"u\x01a1",          L"\x01b0\x01a1" },     // uơ → ươ
    { L"\x01b0" L"a",      L"u\x0103" },           // ưa → uă
    { L"\x01b0o",          L"\x01b0\x01a1" },       // ưo → ươ
};
```

**Step 3: Apply in TryAddC2**

In `telex_engine.cpp`, inside TryAddC2 after validating C2:
```cpp
bool TelexEngine::TryAddC2(wchar_t c) {
    if (_v.empty() && _c1 != L"gi") return false;

    std::wstring candidate = _c2 + c;
    if (TelexData::ValidC2.count(candidate)) {
        // Apply vowel adjustments for C2 context
        for (const auto& t : TelexData::WvC2Transitions) {
            if (_v == t.from) {
                _v = t.to;
                break;
            }
        }
        _c2 = candidate;
        return true;
    }
    return false;
}
```

**Step 4: Run tests, verify pass**

**Step 5: Commit**

```
feat: add vowel adjustments on C2 (transitions_wv_c2)
```

---

### Task 5: Max Word Length

VietType caps at 10 characters. Prevents runaway accumulation.

**Files:**
- Modify: `src/engine/telex_engine.cpp` (PushChar)
- Modify: `tests/test_edge_cases.cpp`

**Step 1: Write failing test**

```cpp
bool test_max_length() { ASSERT_WSTR_EQ(commit("abcdefghijk"), L"abcdefghijk"); return true; }  // >10 → invalid
```

**Step 2: Add check at top of PushChar**

```cpp
TelexStates TelexEngine::PushChar(wchar_t c) {
    // ... existing checks ...

    // Max syllable length
    if (_keyBuffer.size() >= 10) {
        _keyBuffer += c;
        _cases.push_back(iswupper(c) ? 1 : 0);
        _state = TelexStates::Invalid;
        return _state;
    }
    // ... rest of function
```

**Step 3: Run tests, verify pass. Make sure existing long words (like "truwowngf" = 9 chars) still work.**

**Step 4: Commit**

```
feat: enforce max word length of 10 characters
```

---

### Task 6: Proper "xoong" Handling

Currently "xoong" produces "xông" (because oo→ô fires, then n+g as C2). Need the reverse transition ô+o→oo so that "xooong" (3 o's) produces "xoong".

But also need VietType's special case: if C2 is already set and we're adding another vowel that makes _v=="oo", invalidate. This prevents malformed states.

This is largely handled by Task 2 (reverse transition). Verify behavior and add test.

**Files:**
- Modify: `tests/test_words.cpp`

**Step 1: Write tests**

```cpp
bool test_xoong()  { ASSERT_WSTR_EQ(commit("xoong"), L"x\x00f4ng"); return true; }    // xoong → xông (normal)
bool test_xooong() { ASSERT_WSTR_EQ(commit("xooong"), L"xoong"); return true; }        // xooong → xoong (reverse)
```

**Step 2: Verify tests pass after Task 2 transitions are in place**

**Step 3: Commit (if needed)**

```
test: add xoong/xooong test cases
```

---

### Task 7: W Transitions After "qu" (restricted)

VietType uses a restricted W table for "qu" context: only u→ư, uo→uơ, uoi→uơi (not the full set). Currently KType uses the same W table for all contexts.

**Files:**
- Modify: `src/engine/telex_data.h` (add WTransitionsQ table)
- Modify: `src/engine/telex_engine.cpp` (TryAddW)
- Modify: `tests/test_words.cpp`

**Step 1: Analyze impact**

The difference: after "qu", W on "ua" should NOT produce "ưa" (because "qưa" is invalid). Currently KType would produce this. With restricted table, only these W transitions are valid after "qu": u→ư, uo→uơ, uoi→uơi.

**Step 2: Write failing test**

```cpp
bool test_quaw()  { ASSERT_WSTR_EQ(commit("quaw"), L"quaw"); return true; }  // qu+a+w → invalid (no breve after qu)
```

**Step 3: Add restricted table and use in TryAddW**

In `telex_data.h`:
```cpp
inline const WTransition WTransitionsQ[] = {
    { L"u",    L"\x01b0" },                      // u → u-horn
    { L"uo",   L"u\x01a1" },                     // uo → u o-horn
    { L"uoi",  L"u\x01a1i" },                    // uoi → u o-horn i
};
```

In `telex_engine.cpp` TryAddW(), select table based on _c1:
```cpp
const auto* wTable = (_c1 == L"qu") ? TelexData::WTransitionsQ : TelexData::WTransitions;
// ... similarly for WA: skip WA entirely if _c1 == "qu"
```

**Step 4: Run tests, verify pass**

**Step 5: Commit**

```
feat: use restricted W transitions after "qu"
```

---

## Deferred (not in scope)

These features are OFF by default in VietType and can be added later:

- **Autocorrect** (default OFF): w-reorder, ie→iê, trailing h/g normalization
- **English dictionary optimization** (Level 1): skip for now, KType users are Vietnamese-focused
- **Backconvert**: needed for TSF composition editing, but separate concern
- **Abbreviation mode**: not relevant for IME
- **TelexComplicated**: `[`/`]` for direct horn input, not standard Telex

## Execution Order

Tasks 1-5 are independent and can be done in any order. Task 6 depends on Task 2. Task 7 is independent.

Recommended: 1 → 2+6 → 3 → 4 → 5 → 7
