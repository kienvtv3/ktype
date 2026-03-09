# VietType Telex Engine Reference

Comprehensive documentation of VietType's engine architecture for KType adaptation.
Source: `D:\Projects\ktype-master\VietType\Telex\`

## 1. State Machine (Telex.h)

### States
- **Valid** — Composition in progress, tones NOT yet applied
- **Invalid** — Bad sequence, can still accept chars
- **Committed** — Valid word with tones applied, terminal
- **CommittedInvalid** — Raw output, terminal
- **BackconvertFailed** — Backconversion failed

### Configuration (TelexConfig)
```cpp
bool oa_uy_tone1 = true;            // tone on 2nd char in oa/oe/uy (new style)
bool accept_separate_dd = true;     // dd→đ even after vowels
bool backspaced_word_stays_invalid = true;
bool autocorrect = false;           // OFF by default
unsigned long optimize_multilang = 1; // English dict level (0-7)
bool allow_abbreviations = true;    // QĐ shortcuts
```

## 2. Core Data Structures (TelexEngine.h)

### Member Variables
- `_keyBuffer` — All typed keys (max 250, max word 10)
- `_c1` — Onset consonant (b, ch, gi, ng, ngh, ph, qu, tr, đ...)
- `_v` — Vowel nucleus (a, â, ê, ô, ơ, ư, ươ, iê, uyê...)
- `_c2` — Coda consonant (c, ch, k, m, n, ng, nh, p, t)
- `_t` — Tone (Z=none, S=sắc, F=huyền, R=hỏi, X=ngã, J=nặng)
- `_toneCount` — Number of tone keys typed
- `_cases` — Uppercase tracking per output char
- `_respos` — Keystroke-to-output position mapping (for backspace replay)

### VInfo & C2Mode
```cpp
struct VInfo { int tonepos; C2Mode c2mode; };
enum C2Mode { Either, MustC2, NoC2 };
```

## 3. PushChar Priority Order

1. **C1 consonant** — if _v empty, accumulate onset
2. **"gi" cluster** — special detection
3. **DD → đ** — d-stroke transition
4. **Vowel transitions** — doubling triggers circumflex (aa→â, iee→iê)
5. **W transitions** — horn (ơ,ư,ươ) then breve (ă,oă,uă)
6. **Tone marks** — s,f,r,x,j,z
7. **C2 consonants** — coda after vowel

**Key**: Tone is LAZY — applied only at Commit(), not during PushChar().

## 4. Vowel Transition Tables

### Telex Doubling (transitions_telex)
```
aa → â         ee → ê         oo → ô
iee → iê      yee → yê       uoo → uô
uee → uê      uaa → uâ       uyee → uyê
aua → âu       aya → ây       eue → êu
oio → ôi       uaya → uây     uoio → uôi
ieue → iêu     yeue → yêu
ô+o → oo       (reverse, for "xoong")
```

### W Transitions (horn)
```
o → ơ          oi → ơi        u → ư
ua → ưa        ui → ưi        uo → ươ
uoi → ươi      uou → ươu      uu → ưu
ưo → ươ        ươ → ươ (identity, prevent double-w undo)
```

### WA Transitions (breve)
```
a → ă          oa → oă        ua → uă
```

### W after "qu" (restricted)
```
u → ư          uo → uơ        uoi → uơi
```

### WA after "qu"
```
a → ă
```

### Vowel adjustments on C2 (transitions_wv_c2)
```
uơ → ươ       ưa → uă        ưo → ươ
```

## 5. Valid Vowel Tables

### valid_v (General) — Complete Table
| Vowel | tonePos | C2Mode | Notes |
|-------|---------|--------|-------|
| a | 0 | Either | |
| ai | 0 | NoC2 | |
| ao | 0 | NoC2 | |
| au | 0 | NoC2 | |
| ay | 0 | NoC2 | |
| e | 0 | Either | |
| eo | 0 | NoC2 | |
| i | 0 | Either | |
| ia | 0 | NoC2 | |
| iu | 0 | NoC2 | |
| iê | 1 | MustC2 | |
| iêu | 1 | NoC2 | |
| o | 0 | Either | |
| oa | 1 | Either | oa_uy affected |
| oai | 1 | NoC2 | |
| oao | 1 | NoC2 | |
| oay | 1 | NoC2 | |
| oe | 1 | Either | oa_uy affected |
| oeo | 1 | NoC2 | |
| oi | 0 | NoC2 | |
| oo | 1 | MustC2 | |
| oă | 1 | MustC2 | |
| u | 0 | Either | |
| ua | 0 | NoC2 | **tonePos=0** (của not cuả) |
| uao | 1 | NoC2 | |
| uay | 1 | NoC2 | |
| ui | 0 | NoC2 | |
| uy | 1 | Either | oa_uy affected |
| uyê | 2 | MustC2 | |
| uyu | 1 | NoC2 | |
| uâ | 1 | MustC2 | |
| uây | 1 | NoC2 | |
| uê | 1 | Either | |
| uô | 1 | MustC2 | |
| uôi | 1 | NoC2 | |
| uă | 1 | MustC2 | |
| uơ | 1 | NoC2 | intermediate W state |
| y | 0 | NoC2 | |
| yê | 1 | MustC2 | |
| yêu | 1 | NoC2 | |
| â | 0 | MustC2 | **bare â invalid** |
| âu | 0 | NoC2 | |
| ây | 0 | NoC2 | |
| ê | 0 | Either | |
| êu | 0 | NoC2 | |
| ô | 0 | Either | |
| ôi | 0 | NoC2 | |
| ă | 0 | MustC2 | |
| ơ | 0 | Either | |
| ơi | 0 | NoC2 | |
| ư | 0 | Either | |
| ưa | 0 | NoC2 | **forbids C2** |
| ưi | 0 | NoC2 | |
| ưu | 0 | NoC2 | |
| ươ | 1 | MustC2 | |
| ươi | 1 | NoC2 | |
| ươu | 1 | NoC2 | |

### valid_v_q (After "qu")
| Vowel | tonePos | C2Mode | Notes |
|-------|---------|--------|-------|
| ua | 1 | Either | tone on 'a' (quá, quán) |
| uai | 1 | NoC2 | |
| uao | 1 | NoC2 | |
| uau | 1 | NoC2 | quạu |
| uay | 1 | NoC2 | |
| ue | 1 | Either | |
| ueo | 1 | NoC2 | |
| ui | 1 | Either | |
| uo | 1 | NoC2 | quọ |
| uy | 1 | Either | |
| uyê | 2 | MustC2 | |
| uâ | 1 | MustC2 | |
| uây | 1 | NoC2 | |
| uê | 1 | Either | |
| uêu | 1 | NoC2 | |
| uô | 1 | Either | |
| uă | 1 | MustC2 | |
| uơ | 1 | Either | |
| uơi | 1 | NoC2 | |
| ươ | 1 | MustC2 | |

### valid_v_gi (After "gi")
- Includes empty string: tonePos=-1 (tone on 'i' in "gi")
- Subset of valid_v (no ươ, iê, yê combos)
- "uô" is Either (not MustC2) after gi

### valid_v_oa_uy (oa_uy_tone1=false override)
Only 3 entries, used when C2 empty AND C1 not "q"/"gi":
| Vowel | tonePos |
|-------|---------|
| oa | 0 | → hòa (old style) |
| oe | 0 | → xòe (old style) |
| uy | 0 | → thùy (old style) |

### FindTable() Logic
```cpp
if (_c1 == "q")     → valid_v_q
else if (_c1 == "gi") → valid_v_gi
else:
    if (_c2.empty() && !oa_uy_tone1)
        try valid_v_oa_uy first
    fallback to valid_v
```

## 6. Commit Validation

1. Validate C1 ∈ valid_c1
2. Validate C2 ∈ valid_c2
3. If C2 is restricted (c/ch/k/p/t) AND tone ∉ {S, J} → Invalid
   - **Exception**: đ onset bypasses restricted C2 check (teencode)
4. FindTable() → get VInfo for _v
5. If MustC2 and no C2 → Invalid
6. If NoC2 and has C2 → Invalid
7. Apply tone to _v[tonePos]

## 7. English Word Optimization (optimize_multilang)

| Level | Features |
|-------|----------|
| 0 | No optimization |
| 1 (default) | wlist_en (~200 words: bars, cheers, airs...) |
| 2 | + wlist_en_2 (~500: ask, boat, door...) |
| 3+ | + InvalidateOnVowelPostTone, InvalidateDoubleTone |

When matched, word is rejected as CommittedInvalid (raw output).
This is what prevents "test"→"tét", "just"→"just", etc.

## 8. Autocorrect (OFF by default)

Only when `autocorrect=true`:
1. W-fixup: wu→ưu, wo→ơ, wuo→ươ
2. IE→IÊ when C1+C2 present and tone S/J
3. Trailing H: ah+s→ách, ah→anh
4. Trailing G/GN: g→ng, gn→ng

## 9. Backspace

Replay-based: removes last keystroke, replays remaining buffer through PushChar.
Uses `_respos` to track which output position each keystroke produced.

## 10. Differences: KType vs VietType

### Already Implemented
- Syllable model: C1 + V + C2 + Tone
- Vowel transitions (doubling)
- W/WA transitions (horn/breve)
- W after "qu" (restricted set)
- WvC2 transitions (vowel adjustment on C2)
- Tone same-key invalidation
- C2Mode constraints (MustC2, NoC2)
- Teencode exception (đ bypasses restricted C2)
- Leading W requires empty C1
- Reverse circumflex (ô+o→oo)
- oa_uy_tone1 config

### Not Yet Implemented
- [ ] `valid_v_q` separate table (KType uses "qu" as C1, different approach)
- [ ] `valid_v_gi` separate table (KType handles gi differently)
- [ ] English dictionary optimization (optimize_multilang)
- [ ] Autocorrect features
- [ ] Abbreviations (QĐ)
- [ ] Backconversion
- [ ] `backspaced_word_stays_invalid` config
- [ ] Missing vowels: oao, oeo, uao, uyu, uơ

### Architectural Differences
- **KType**: "qu" consumed into C1, _v starts after 'u'
- **VietType**: "q" is C1, _v includes 'u' → needs separate valid_v_q table
- Both approaches produce correct output but with different internal representations

## 11. Design Decision: _undoChar vs _respos (2026-03-09)

### Context
VietType uses a per-keystroke `_respos[]` bitmask system where each key gets flags like
`ResposDoubleUndo`, `ResposTransitionV`, `ResposInvalidate`, `ResposExpunged`. This enables:
- `RetrieveRaw()` to automatically exclude DoubleUndo-marked chars from output
- Backspace to pop 2 chars intelligently when undoing a transition
- Correct handling of edge cases like "toool" → "tool"

KType uses `_undoChar` — a single wchar_t tracking the last transition trigger. Simpler but
less capable.

### What KType handles correctly with _undoChar
- Vowel doubling undo: aaa→"aa", eee→"ee", dataa→"data" (159 tests pass)
- W undo: oww→"ow", uww→"uw", aww→"aw" (InvalidateAndPopBack style)
- Reverse transitions: ôo→oo for "xoong", "thòng" patterns
- All VietType compat tests ported from TestTelex.cpp pass

### Known gaps (would require _respos to fix)
- "toool" → KType outputs "toool", VietType outputs "tool"
  (VietType's _respos filters DoubleUndo char from ALL output paths)
- Backspace from undo state: VietType pops 2 intelligently, KType replays
  (same result but different mechanism; replay is O(n) vs O(1))
- Any future case where per-keystroke metadata is needed

### What we have NOT verified
- Full coverage of VietType's TestTelex.cpp and TestTelexComplicated.cpp
  (only ported a subset of double-key and backspace tests)
- Systematic comparison of all behavior — tested specific cases only
- Whether other edge cases exist beyond "toool" pattern

### Decision
Keep `_undoChar` for now. Rationale:
1. Code is significantly simpler and more readable than _respos bitmasks
2. All 159 tests pass including ported VietType compat tests
3. Known gap ("toool") is a minor edge case unlikely in real typing
4. Replay-based backspace is correct even if not optimal (max 10 chars)

### When to reconsider
- If users report incorrect behavior that _undoChar cannot handle
- If we need smarter backspace (e.g., for autocorrect features)
- If porting remaining VietType tests reveals more gaps
- Action item: port ALL tests from TestTelex.cpp and TestTelexComplicated.cpp
  to get full coverage before claiming parity
