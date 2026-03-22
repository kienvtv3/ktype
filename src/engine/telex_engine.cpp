#include "telex.h"
#include "telex_data.h"
#include "wordlist.h"
#include <algorithm>
#include <cassert>
#include <cwctype>

namespace KType {

static const auto& ToneMap() {
    static auto m = TelexData::MakeToneMap();
    return m;
}

static const auto& VowelMap() {
    static auto m = TelexData::MakeVowelMap();
    return m;
}

static const auto& VowelTransitionMap() {
    static auto m = TelexData::MakeVowelTransitionMap();
    return m;
}

static const auto& VowelTransitionPrefixSet() {
    static auto s = TelexData::MakeVowelTransitionPrefixSet();
    return s;
}

static const auto& ValidVowelPrefixSet() {
    static auto s = TelexData::MakeValidVowelPrefixSet();
    return s;
}

static const auto& WTransitionFromSet() {
    static auto s = TelexData::MakeWTransitionFromSet();
    return s;
}

TelexEngine::TelexEngine(const TelexConfig& config) : _config(config) {
    Reset();
}

void TelexEngine::Reset() {
    _state = TelexStates::Valid;
    _keyBuffer.clear();
    _c1.clear();
    _v.clear();
    _c2.clear();
    _t = Tones::Z;
    _cases.clear();
    _respos.clear();
    _respos_current = 0;
    _hasD = false;
    _hasW = false;
    _leadingW = false;
    _hasAbbreviation = false;
    _result.clear();
    assert(CheckInvariants());
}

bool TelexEngine::AcceptsChar(wchar_t c) {
    wchar_t lc = (wchar_t)towlower(c);
    return (lc >= L'a' && lc <= L'z');
}

TelexEngine::CharType TelexEngine::Classify(wchar_t c) const {
    wchar_t lc = (wchar_t)towlower(c);

    if (lc == L'w') return CharType::TransitionW;

    // Tone keys: count as tone if we have vowels OR "gi" C1 (where 'i' acts as vowel)
    if (TelexData::IsToneKey(lc) && (!_v.empty() || _c1 == L"gi")) {
        return CharType::ToneMark;
    }

    if (TelexData::IsVowel(lc)) return CharType::Vowel;

    // 'd' is special: first d is consonant, second d makes d-stroke
    if (lc == L'd') return CharType::DoubleD;

    if (TelexData::IsConsonant(lc)) return CharType::Consonant;

    return CharType::Invalid;
}

void TelexEngine::Invalidate() {
    _respos.push_back(_respos_current | ResposInvalidate);
    _state = TelexStates::Invalid;
}

void TelexEngine::InvalidateAndPopBack(wchar_t c) {
    wchar_t lc = (wchar_t)towlower(c);
    if (_keyBuffer.length() > 1 && lc == (wchar_t)towlower(_keyBuffer[_keyBuffer.size() - 2])) {
        _respos.push_back(_respos_current | ResposDoubleUndo);
    } else {
        _respos.push_back(_respos_current | ResposInvalidate);
    }
    _state = TelexStates::Invalid;
}

TelexStates TelexEngine::PushChar(wchar_t c) {
    if (_state != TelexStates::Valid && _state != TelexStates::Invalid) {
        return _state;
    }

    // Max syllable length (VietType: MaxLength = 10)
    if (_state == TelexStates::Valid && _keyBuffer.size() >= 10) {
        _state = TelexStates::Invalid;
    }

    _keyBuffer += c;

    if (_state == TelexStates::Invalid) {
        Invalidate();
        return _state;
    }

    wchar_t lc = (wchar_t)towlower(c);
    bool ccase = iswupper(c) != 0;
    CharType type = Classify(c);

    bool handled = false;
    switch (type) {
    case CharType::DoubleD:
        handled = TryAddD(lc, ccase);
        // Abbreviation fallback: 'd' that can't be handled as DoubleD chains to C1
        if (!handled && _config.allow_abbreviations && !_c1.empty() &&
            _v.empty() && _c2.empty()) {
            _c1 += L'd';
            _cases.push_back(ccase);
            _respos.push_back(_respos_current++);
            handled = true;
        }
        break;
    case CharType::Consonant:
        // Allow C2 after "gi" (where 'i' is stored in C1 but acts as vowel)
        if (_v.empty() && _c1 != L"gi") {
            handled = TryAddC1(lc, ccase);
            // Abbreviation chaining: allow consonant after đ (ddc→đc)
            // and "vn" prefix for VNĐ (vndd→vnđ)
            if (!handled && _config.allow_abbreviations && _c2.empty()) {
                bool allowChain = _hasD ||                      // after đ: ddc→đc
                    (_c1 == L"v" && lc == L'n');                // vn prefix for VNĐ
                if (allowChain) {
                    _c1 += lc;
                    if (_hasD) _hasAbbreviation = true;
                    _cases.push_back(ccase);
                    _respos.push_back(_respos_current++);
                    handled = true;
                }
            }
        } else {
            handled = TryAddC2(lc, ccase);
            if (!handled) {
                // Permissive: accept non-C2 consonants (VietType defers validation to Commit)
                // But DON'T override if the char IS a valid C2 that was rejected (e.g., tone restriction)
                if (!TelexData::ValidC2.count(std::wstring(1, lc))) {
                    if (_c2.empty()) {
                        // Apply WvC2 transitions when first C2 is being added
                        for (const auto& t : TelexData::WvC2Transitions) {
                            if (_v == t.from) { _v = t.to; break; }
                        }
                    }
                    _c2 += lc;
                    _cases.push_back(ccase);
                    _respos.push_back(_respos_current++);
                    handled = true;
                }
            }
        }
        break;
    case CharType::Vowel:
        // Special: 'u' after 'q' forms 'qu' consonant cluster
        if (lc == L'u' && _c1 == L"q" && _v.empty()) {
            _c1 = L"qu";
            _cases.push_back(ccase);
            _respos.push_back(_respos_current++);
            handled = true;
            break;
        }
        // Special: 'i' after 'g' forms 'gi' consonant cluster
        if (lc == L'i' && _c1 == L"g" && _v.empty()) {
            _c1 = L"gi";
            _cases.push_back(ccase);
            _respos.push_back(_respos_current++);
            handled = true;
            break;
        }
        handled = TryAddVowel(lc, ccase);
        break;
    case CharType::ToneMark:
        handled = TryAddTone(lc);
        break;
    case CharType::TransitionW:
        handled = TryAddW(lc, ccase);
        break;
    case CharType::Invalid:
        Invalidate();
        return _state;
    }

    if (!handled) {
        Invalidate();
    }

    assert(CheckInvariants());
    return _state;
}

bool TelexEngine::TryAddD(wchar_t /*c*/, bool ccase) {
    // First 'd': only before any vowels/C2
    if (_c1.empty() && _v.empty() && _c2.empty()) {
        _c1 = L"d";
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++);
        return true;
    }
    // Second 'd': convert to đ
    if (_c1 == L"d" && !_hasD) {
        if (_config.accept_separate_dd || (_v.empty() && _c2.empty())) {
            _c1 = L"\x0111";
            _hasD = true;
            _respos.push_back(0 | ResposTransitionC1);
            return true;
        }
    }
    // Abbreviation: dd→đ after other consonants (e.g., "qdd"→"qđ")
    if (_config.allow_abbreviations && !_c1.empty() && _c1.back() == L'd' &&
        _v.empty() && _c2.empty()) {
        _c1.back() = L'\x0111';
        _hasD = true;
        _hasAbbreviation = true;
        _respos.push_back(static_cast<unsigned int>(_c1.size() - 1) | ResposTransitionC1);
        return true;
    }
    return false;
}

bool TelexEngine::TryAddC1(wchar_t c, bool ccase) {
    if (!_v.empty() || !_c2.empty()) {
        return false;
    }

    if (_c1.empty()) {
        _c1 = std::wstring(1, c);
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++);
        return true;
    }

    // Try extending C1: ch, gh, gi, kh, ng, ngh, nh, ph, th, tr, qu
    if (TelexData::CanContinueC1(_c1, c)) {
        std::wstring candidate = _c1 + c;
        if (TelexData::ValidC1.count(candidate)) {
            _c1 = candidate;
            _cases.push_back(ccase);
            _respos.push_back(_respos_current++);
            return true;
        }
    }

    // Special: 'q' + 'u' -> qu
    if (_c1 == L"q" && c == L'u') {
        _c1 = L"qu";
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++);
        return true;
    }

    return false;
}

bool TelexEngine::TryAddVowel(wchar_t c, bool ccase) {
    std::wstring candidate = _v + c;

    // If C2 contains an invalid consonant (permissively accepted), reject vowel addition.
    // The word will commit as raw anyway, so preview should match commit output.
    // (e.g., "kobo": C2="b" invalid → don't let oo→ô transition produce misleading "kôb")
    if (!_c2.empty() && !TelexData::ValidC2.count(_c2)) {
        return false;
    }

    // 1. Check if candidate matches a vowel transition (apply it)
    // When C2 is present and ôo reverse transition would fire, use DoubleUndo instead
    // so the extra 'o' undoes the oo→ô and gets filtered from output (photoo→photo)
    if (!_c2.empty() && c == L'o' && candidate == L"\x00f4o" &&
        !_respos.empty() && (_respos.back() & ResposTransitionV)) {
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++ | ResposDoubleUndo);
        _state = TelexStates::Invalid;
        return true;
    }
    {
        auto vtIt = VowelTransitionMap().find(candidate);
        if (vtIt != VowelTransitionMap().end()) {
            auto before = candidate.size(); // input size (VietType: _v after push)
            _v = vtIt->second;
            auto after = _v.size();         // output size after transition

            // Check for DoubleUndo: same char after a transition → stays Valid but marked
            if (_keyBuffer.size() > 1 &&
                !_respos.empty() && (_respos.back() & ResposTransitionV) &&
                c == (wchar_t)towlower(_keyBuffer[_keyBuffer.size() - 2])) {
                // Reverse transition (e.g., "ooo": ô+o → oo, 3rd 'o' is DoubleUndo)
                _cases.push_back(ccase);
                _respos.push_back(_respos_current++ | ResposDoubleUndo);
            } else if (after < before) {
                // Transition consumed a char (e.g., "oo" → "ô": 2 chars → 1)
                // Compute offset: position of first difference between input and result
                int offset = 0;
                for (size_t k = 0; k < after; k++) {
                    if (candidate[k] != _v[k]) break;
                    offset++;
                }
                _respos.push_back(static_cast<unsigned int>(_c1.size() + offset) | ResposTransitionV);
            } else if (after == before) {
                // Transition replaced in-place (size unchanged)
                _cases.push_back(ccase);
                _respos.push_back(_respos_current++ | ResposTransitionV);
            }
            return true;
        }
    }

    // 2. No transition matched. If previous was a transition and same char → DoubleUndo + Invalid
    if (!_respos.empty() && (_respos.back() & ResposTransitionV) &&
        _keyBuffer.size() > 1 &&
        c == (wchar_t)towlower(_keyBuffer[_keyBuffer.size() - 2])) {
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++ | ResposDoubleUndo);
        _state = TelexStates::Invalid;
        return true;
    }

    // 3. If C2 is already present and no transition matched, reject
    if (!_c2.empty()) {
        return false;
    }

    // 4. Check if candidate is a prefix of a transition (keep accumulating)
    if (VowelTransitionPrefixSet().count(candidate)) {
        _v = candidate;
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++);
        return true;
    }

    // 5. Check if candidate is a valid vowel
    if (VowelMap().count(candidate)) {
        _v = candidate;
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++);
        return true;
    }

    // 6. Check if candidate is a prefix of a valid vowel
    if (ValidVowelPrefixSet().count(candidate)) {
        _v = candidate;
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++);
        return true;
    }

    // 7. Check if candidate can be W-transformed into something valid
    if (WTransitionFromSet().count(candidate)) {
        _v = candidate;
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++);
        return true;
    }

    // 8. Permissive: accept any vowel when C2 is empty (VietType deferred validation)
    // VietType always pushes vowels to _v, deferring validation to Commit
    _v = candidate;
    _cases.push_back(ccase);
    _respos.push_back(_respos_current++);
    return true;
}

bool TelexEngine::TryAddC2(wchar_t c, bool ccase) {
    if (_v.empty() && _c1 != L"gi") return false;

    std::wstring candidate = _c2 + c;

    if (TelexData::ValidC2.count(candidate)) {
        // VietType tone restriction: restricted C2 with non-S/J tone → reject
        // Exception: đ onset bypasses this (teencode)
        if (_c1 != L"\x0111" && _t != Tones::Z && _t != Tones::S && _t != Tones::J) {
            if (TelexData::RestrictedC2.count(std::wstring(1, c))) {
                return false;
            }
        }
        // Apply vowel adjustments for C2 context (only if W was used)
        if (_hasW) {
            for (const auto& t : TelexData::WvC2Transitions) {
                if (_v == t.from) {
                    _v = t.to;
                    break;
                }
            }
        }
        _c2 = candidate;
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++);
        return true;
    }

    return false;
}

bool TelexEngine::TryAddTone(wchar_t c) {
    if (_v.empty() && _c1 != L"gi") return false;

    Tones newTone = TelexData::GetTone(c);

    // Same tone pressed again → InvalidateAndPopBack (VietType behavior)
    if (_t == newTone && newTone != Tones::Z) {
        InvalidateAndPopBack(c);
        return true;
    }

    // Z on Z is redundant — reject so the char is handled as consonant/invalid
    // (e.g., "size": 'z' after untoned 'i' should not be consumed)
    if (newTone == Tones::Z && _t == Tones::Z) {
        return false;
    }

    _t = newTone;
    _respos.push_back(ResposTone);
    return true;
}

bool TelexEngine::TryAddW(wchar_t /*c*/, bool ccase) {
    // Leading W (empty vowel): only when C1 is also empty
    if (_v.empty()) {
        if (_c1.empty()) {
            _v = L"\x01b0"; // u-horn
            _hasW = true;
            _leadingW = true;
            _cases.push_back(ccase);
            _respos.push_back(_respos_current++);
            return true;
        }
        // Special: W after "qu" → extract 'u' from C1, transform to ư in V
        // VietType keeps C1="q" with 'u' in V; KType has C1="qu" with V empty
        if (_c1 == L"qu") {
            _c1 = L"q";
            _v = L"\x01b0"; // u-horn (W transition on the extracted 'u')
            _hasW = true;
            // _cases: move last entry (the 'u' case) to be V's case — it's already there
            // _respos: the 'u' entry stays at its position, 'w' adds TransitionW
            _respos.push_back(static_cast<unsigned int>(_c1.size()) | ResposTransitionW);
            return true;
        }
        return false;
    }

    bool isQu = (_c1 == L"qu");

    // 1. Try W transitions (full or restricted based on C1)
    //    VietType w_mode: allow self-transitions only when _c2 empty && prev not TransW
    const auto* wBegin = isQu ? std::begin(TelexData::WTransitionsQ) : std::begin(TelexData::WTransitions);
    const auto* wEnd = isQu ? std::end(TelexData::WTransitionsQ) : std::end(TelexData::WTransitions);
    for (auto it = wBegin; it != wEnd; ++it) {
        if (_v == it->from) {
            // Self-transition: only allow when _c2 is empty
            if (it->from == it->to && !_c2.empty()) continue;
            // Check w_mode guard: don't allow W transition if previous was already W
            if (!_respos.empty() && (_respos.back() & ResposTransitionW)) {
                continue;
            }
            _v = it->to;
            _hasW = true;
            if (!_c2.empty()) {
                // Apply WvC2 transitions if C2 already present
                for (const auto& wvc2 : TelexData::WvC2Transitions) {
                    if (_v == wvc2.from) {
                        _v = wvc2.to;
                        break;
                    }
                }
            }
            // Compute offset: position of first difference between from and to
            int offset = 0;
            for (size_t k = 0; k < it->to.size(); k++) {
                if (it->from[k] != it->to[k]) break;
                offset++;
            }
            _respos.push_back(static_cast<unsigned int>(_c1.size() + offset) | ResposTransitionW);
            return true;
        }
    }

    // 2. Try WA transitions
    const auto* waBegin = isQu ? std::begin(TelexData::WATransitionsQ) : std::begin(TelexData::WATransitions);
    const auto* waEnd = isQu ? std::end(TelexData::WATransitionsQ) : std::end(TelexData::WATransitions);
    for (auto it = waBegin; it != waEnd; ++it) {
        if (_v == it->from) {
            // Check w_mode guard
            if (!_respos.empty() && (_respos.back() & ResposTransitionW)) {
                continue;
            }
            _v = it->to;
            _hasW = true;
            // Compute offset: position of first difference between from and to
            int waOffset = 0;
            for (size_t k = 0; k < it->to.size(); k++) {
                if (it->from[k] != it->to[k]) break;
                waOffset++;
            }
            _respos.push_back(static_cast<unsigned int>(_c1.size() + waOffset) | ResposTransitionW);
            return true;
        }
    }

    // No forward match → InvalidateAndPopBack (VietType behavior)
    InvalidateAndPopBack((wchar_t)L'w');
    return true;
}

int TelexEngine::GetTonePosition() const {
    auto it = VowelMap().find(_v);
    if (it != VowelMap().end()) {
        int pos = it->second.tonePos;
        if (!_config.oa_uy_tone1) {
            if (_v == L"oa" || _v == L"oe" || _v == L"uy") {
                pos = 0;
            }
        }
        return pos;
    }

    // Fallback for raw vowel forms
    if (_v.size() == 1) return 0;
    if (_v.size() >= 2) return 1;
    return 0;
}

std::wstring TelexEngine::ApplyTone(const std::wstring& vowel, Tones tone, int pos) const {
    if (tone == Tones::Z || pos < 0 || pos >= (int)vowel.size()) {
        return vowel;
    }

    std::wstring result = vowel;
    wchar_t base = vowel[pos];
    auto it = ToneMap().find(base);
    if (it != ToneMap().end()) {
        result[pos] = TelexData::ToneTable[it->second].toned[(int)tone];
    }
    return result;
}

void TelexEngine::ApplyCases(std::wstring& result) const {
    // _cases is per-output-char, direct 1:1 mapping with result
    for (size_t i = 0; i < _cases.size() && i < result.size(); i++) {
        if (_cases[i]) {
            result[i] = TelexData::VnToUpper(result[i]);
        }
    }
}

// Shared validation: returns true if the current state would definitely produce
// raw output on Commit. Used by both Peek() and Commit() to avoid duplicating
// validation logic. Only skips requiresC2 check (user might still type C2).
bool TelexEngine::IsDefinitelyInvalid() const {
    if (_state == TelexStates::Invalid) return true;

    // English wordlist
    if (_config.optimize_multilang >= 1) {
        std::wstring lower = _keyBuffer;
        for (auto& ch : lower) ch = (wchar_t)towlower(ch);
        if (WordListEn().count(lower)) return true;
    }

    // "gi" fixup for validation (local copies, non-mutating)
    std::wstring c1 = _c1, v = _v;
    if (c1 == L"gi" && v.empty()) { c1 = L"g"; v = L"i"; }

    // Invalid C1 (skip for abbreviations like đc, qđ — handled by Commit)
    if (!c1.empty() && !TelexData::ValidC1.count(c1)) {
        bool isAbbreviation = v.empty() && _c2.empty() &&
            _hasAbbreviation && _config.allow_abbreviations;
        if (!isAbbreviation) return true;
    }

    // Vowel validation
    if (!v.empty()) {
        auto it = VowelMap().find(v);
        if (it != VowelMap().end()) {
            // Valid vowel — check C2Mode forbidsC2 (e.g., ưi+n in "win")
            // Only forbidsC2 is checked here; requiresC2 is Commit-only
            // since user might still be typing the coda.
            if (it->second.forbidsC2 && !_c2.empty()) return true;
        } else {
            // Not a valid vowel — allow if it's a prefix being built
            if (!VowelTransitionPrefixSet().count(v) &&
                !ValidVowelPrefixSet().count(v) &&
                !WTransitionFromSet().count(v)) {
                return true;
            }
        }
    }

    // Invalid C2
    if (!_c2.empty() && !TelexData::ValidC2.count(_c2)) return true;

    // Restricted C2 tone (c, ch, k, p, t only allow sắc or nặng)
    // Exception: đ onset bypasses this (teencode)
    if (!_c2.empty() && TelexData::RestrictedC2.count(_c2) && c1 != L"\x0111") {
        if (_t != Tones::Z && _t != Tones::S && _t != Tones::J) return true;
    }

    return false;
}

TelexStates TelexEngine::Commit() {
    if (_state == TelexStates::Committed || _state == TelexStates::CommittedInvalid) {
        return _state;
    }

    if (_keyBuffer.empty()) {
        _result.clear();
        _state = TelexStates::Committed;
        return _state;
    }

    if (_state == TelexStates::Invalid) {
        _result = RetrieveRaw();
        _state = TelexStates::CommittedInvalid;
        return _state;
    }

    // Abbreviation early commit: consonant-only word with đ transition
    // Must check before IsDefinitelyInvalid() since abbreviations like "đc"
    // have invalid C1 but are intentionally valid.
    if (_v.empty() && _c2.empty() && _hasAbbreviation && _config.allow_abbreviations) {
        bool hasC1Transition = std::any_of(_respos.begin(), _respos.end(),
            [](unsigned int rp) { return (rp & ResposTransitionC1) != 0; });
        if (hasC1Transition) {
            _result = _c1;
            ApplyCases(_result);
            _state = TelexStates::Committed;
            return _state;
        }
    }

    // Shared validation (invalid state, English wordlist, C1, vowel, C2, restricted tone)
    if (IsDefinitelyInvalid()) {
        _result = RetrieveRaw();
        _state = TelexStates::CommittedInvalid;
        return _state;
    }

    // "gi" fixup: if C1 is "gi" and V is empty, move 'i' from C1 to V
    if (_c1 == L"gi" && _v.empty()) {
        _c1 = L"g";
        _v = L"i";
    }

    // Vowel validation: must be in VowelMap, and requiresC2 must be satisfied
    // (forbidsC2 is already checked by IsDefinitelyInvalid())
    if (!_v.empty()) {
        auto it = VowelMap().find(_v);
        if (it == VowelMap().end()) {
            _result = RetrieveRaw();
            _state = TelexStates::CommittedInvalid;
            return _state;
        }
        // requiresC2: skip for standalone single-char vowels (â, ă)
        // to allow typing Vietnamese alphabet letters
        bool isStandaloneLetter = _c1.empty() && _c2.empty() && _v.size() == 1;
        if (it->second.requiresC2 && _c2.empty() && !isStandaloneLetter) {
            _result = RetrieveRaw();
            _state = TelexStates::CommittedInvalid;
            return _state;
        }
    }

    // Build result
    int tonePos = GetTonePosition();
    std::wstring tonedV = ApplyTone(_v, _t, tonePos);

    _result = _c1 + tonedV + _c2;
    ApplyCases(_result);

    _state = TelexStates::Committed;
    assert(CheckInvariants());
    return _state;
}

TelexStates TelexEngine::Cancel() {
    _result = RetrieveRaw();
    _state = TelexStates::CommittedInvalid;
    assert(CheckInvariants());
    return _state;
}

TelexStates TelexEngine::Backspace() {
    if (_keyBuffer.empty()) {
        return _state;
    }

    [[maybe_unused]] auto prevState = _state;
    std::wstring buf(_keyBuffer);
    std::vector<unsigned int> rp(_respos);

    if (_state == TelexStates::Invalid) {
        // Invalid backspace: use _respos to determine what to remove
        Reset();
        if (!rp.empty() && (rp.back() & ResposDoubleUndo)) {
            buf.pop_back(); // pop the DoubleUndo char
        }
        if (!buf.empty()) {
            buf.pop_back(); // pop the actual last char
        }
        // Replay ALL remaining chars (VietType: backspaced_word_stays_invalid=false)
        for (size_t i = 0; i < buf.size(); i++) {
            PushChar(buf[i]);
        }
        assert(CheckInvariants());
        return _state;
    }

    if (_state != TelexStates::Valid) {
        return _state;
    }

    // VietType: if can't find tone position and tone is set, simple replay without last char
    // Exception: "gi" with empty V — tone is on 'i' in C1, position IS findable
    if (_t != Tones::Z && !_v.empty() && !VowelMap().count(_v)) {
        Reset();
        if (!buf.empty()) {
            buf.pop_back();
        }
        for (auto c : buf) {
            PushChar(c);
        }
        assert(CheckInvariants());
        assert(prevState != TelexStates::Valid || _state == TelexStates::Valid);
        return _state;
    }

    // Valid state backspace: use _respos for intelligent deletion
    // Determine which output position to delete (the last one)
    auto outputSize = static_cast<unsigned int>(_c1.size() + _v.size() + _c2.size());
    if (outputSize == 0) {
        Reset();
        return _state;
    }
    auto toDelete = outputSize - 1;

    // Check if tone position will be removed
    auto it = VowelMap().find(_v);
    bool toneWasReset = false;
    if (it != VowelMap().end() && _t != Tones::Z) {
        int tonePos = it->second.tonePos;
        if (!_config.oa_uy_tone1 && (_v == L"oa" || _v == L"oe" || _v == L"uy")) {
            tonePos = 0;
        }
        toneWasReset = (_c1.size() + tonePos >= toDelete);
    } else if (_t != Tones::Z && _c1 == L"gi" && _v.empty()) {
        // "gi" special: tone is on 'i' at position C1.size()-1
        toneWasReset = (_c1.size() - 1 >= toDelete);
    } else if (_t != Tones::Z) {
        // Can't find tone position → treat as tone reset
        toneWasReset = true;
    }

    Reset();

    // Expunge tones if tone position was deleted
    if (toneWasReset) {
        for (size_t i = 0; i < rp.size(); i++) {
            if (rp[i] & ResposTone) {
                rp[i] = (rp[i] & ResposMask) | ResposExpunged;
            }
        }
    }

    // Expunge transitions that reference deleted positions
    for (size_t i = 0; i < rp.size(); i++) {
        if ((rp[i] & ResposDoubleUndo) && (rp[i] & ResposMask) >= toDelete) {
            // DoubleUndo at/after deleted position: also expunge the transition before it
            if (i > 0 && (rp[i - 1] & ~ResposMask)) {
                rp[i - 1] = (rp[i - 1] & ResposMask) | ResposExpunged;
            }
        }
    }

    // Replay only non-expunged chars with position < toDelete
    for (size_t i = 0; i < buf.size(); i++) {
        if (!(rp[i] & ResposExpunged) && (rp[i] & ResposMask) < toDelete) {
            PushChar(buf[i]);
        }
    }

    assert(CheckInvariants());
    assert(prevState != TelexStates::Valid || _state == TelexStates::Valid);
    return _state;
}

std::wstring TelexEngine::Retrieve() const {
    if (_state == TelexStates::Committed || _state == TelexStates::CommittedInvalid) {
        return _result;
    }
    return RetrieveRaw();
}

std::wstring TelexEngine::RetrieveRaw() const {
    // Filter out DoubleUndo-marked chars from _keyBuffer
    std::wstring result;
    for (size_t i = 0; i < _keyBuffer.size(); i++) {
        if (i < _respos.size() && (_respos[i] & ResposDoubleUndo)) {
            continue; // skip DoubleUndo chars
        }
        result.push_back(_keyBuffer[i]);
    }
    return result;
}

std::wstring TelexEngine::Peek() const {
    // Shared validation: if Commit would reject, Peek shows raw too
    if (IsDefinitelyInvalid()) {
        return RetrieveRaw();
    }

    if (_v.empty() && _c1.empty()) {
        return RetrieveRaw();
    }

    // Preview: show current composition
    std::wstring preview = _c1 + _v + _c2;

    // Apply tone preview if we have vowels
    if (!_v.empty() && _t != Tones::Z) {
        auto it = VowelMap().find(_v);
        int pos = 0;
        if (it != VowelMap().end()) {
            pos = it->second.tonePos;
            if (!_config.oa_uy_tone1) {
                if (_v == L"oa" || _v == L"oe" || _v == L"uy") {
                    pos = 0;
                }
            }
        } else if (_v.size() >= 2) {
            pos = 1;
        }
        std::wstring tonedV = ApplyTone(_v, _t, pos);
        preview = _c1 + tonedV + _c2;
    }
    // Special: "gi" + tone but no vowel -> apply tone to 'i' in preview
    else if (_c1 == L"gi" && _v.empty() && _t != Tones::Z) {
        std::wstring tempV = L"i";
        std::wstring tonedV = ApplyTone(tempV, _t, 0);
        preview = L"g" + tonedV + _c2;
    }

    ApplyCases(preview);
    return preview;
}

bool TelexEngine::CheckInvariants() const {
    // 1. If _keyBuffer empty: state must be Valid/Committed/CommittedInvalid;
    //    C1/V/C2 empty; tone Z; cases/respos empty; _respos_current 0
    if (_keyBuffer.empty()) {
        if (_state != TelexStates::Valid && _state != TelexStates::Committed &&
            _state != TelexStates::CommittedInvalid)
            return false;
        if (!(_c1.empty() && _v.empty() && _c2.empty()))
            return false;
        if (_t != Tones::Z)
            return false;
        if (!(_cases.empty() && _respos.empty() && _respos_current == 0))
            return false;
    }

    // 2. No ResposExpunged in _respos
    if (std::any_of(_respos.begin(), _respos.end(),
        [](unsigned int r) { return (r & ResposExpunged) != 0; }))
        return false;

    // 3. If Valid or Invalid: c1.size() + v.size() + c2.size() <= keyBuffer.size()
    if (_state == TelexStates::Valid || _state == TelexStates::Invalid) {
        if (_c1.size() + _v.size() + _c2.size() > _keyBuffer.size())
            return false;
    }

    // 4. If Valid: keyBuffer.size() == respos.size()
    if (_state == TelexStates::Valid) {
        if (_keyBuffer.size() != _respos.size())
            return false;
    }

    // 5. If Valid or Committed: c1.size() + v.size() + c2.size() == cases.size()
    if (_state == TelexStates::Valid || _state == TelexStates::Committed) {
        if (_c1.size() + _v.size() + _c2.size() != _cases.size())
            return false;
    }

    return true;
}

} // namespace KType
