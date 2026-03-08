#include "telex.h"
#include "telex_data.h"
#include <algorithm>
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
    _hasD = false;
    _result.clear();
}

bool TelexEngine::AcceptsChar(wchar_t c) {
    wchar_t lc = (wchar_t)towlower(c);
    return (lc >= L'a' && lc <= L'z');
}

TelexEngine::CharType TelexEngine::Classify(wchar_t c) const {
    wchar_t lc = (wchar_t)towlower(c);

    if (lc == L'w') return CharType::TransitionW;

    // Tone keys: only count as tone if we have vowels
    if (TelexData::IsToneKey(lc) && !_v.empty()) {
        return CharType::ToneMark;
    }

    if (TelexData::IsVowel(lc)) return CharType::Vowel;

    // 'd' is special: first d is consonant, second d makes đ
    if (lc == L'd') return CharType::DoubleD;

    if (TelexData::IsConsonant(lc)) return CharType::Consonant;

    return CharType::Invalid;
}

TelexStates TelexEngine::PushChar(wchar_t c) {
    if (_state != TelexStates::Valid && _state != TelexStates::Invalid) {
        return _state;
    }

    _keyBuffer += c;
    _cases.push_back(iswupper(c) ? 1 : 0);

    if (_state == TelexStates::Invalid) {
        return _state; // once invalid, stay invalid
    }

    wchar_t lc = (wchar_t)towlower(c);
    CharType type = Classify(c);

    bool handled = false;
    switch (type) {
    case CharType::DoubleD:
        handled = TryAddD(lc);
        break;
    case CharType::Consonant:
        if (_v.empty()) {
            handled = TryAddC1(lc);
        } else {
            handled = TryAddC2(lc);
        }
        break;
    case CharType::Vowel:
        // Special: 'u' after 'q' forms 'qu' consonant cluster
        if (lc == L'u' && _c1 == L"q" && _v.empty()) {
            _c1 = L"qu";
            handled = true;
            break;
        }
        // Special: 'i' after 'g' forms 'gi' consonant cluster
        if (lc == L'i' && _c1 == L"g" && _v.empty()) {
            _c1 = L"gi";
            handled = true;
            break;
        }
        // If we already have C2, this is invalid
        if (!_c2.empty()) {
            _state = TelexStates::Invalid;
            return _state;
        }
        handled = TryAddVowel(lc);
        break;
    case CharType::ToneMark:
        handled = TryAddTone(lc);
        break;
    case CharType::TransitionW:
        handled = TryAddW(lc);
        break;
    case CharType::Invalid:
        _state = TelexStates::Invalid;
        return _state;
    }

    if (!handled) {
        _state = TelexStates::Invalid;
    }

    return _state;
}

bool TelexEngine::TryAddD(wchar_t /*c*/) {
    if (_v.empty() && _c2.empty()) {
        if (_c1.empty()) {
            // First 'd' → regular consonant
            _c1 = L"d";
            return true;
        }
        if (_c1 == L"d" && !_hasD) {
            // Second 'd' → đ
            _c1 = L"\x0111"; // đ
            _hasD = true;
            return true;
        }
    }
    return false;
}

bool TelexEngine::TryAddC1(wchar_t c) {
    if (!_v.empty() || !_c2.empty()) {
        return false;
    }

    if (_c1.empty()) {
        _c1 = std::wstring(1, c);
        return true;
    }

    // Try extending C1: ch, gh, gi, kh, ng, ngh, nh, ph, th, tr, qu
    if (TelexData::CanContinueC1(_c1, c)) {
        std::wstring candidate = _c1 + c;
        if (TelexData::ValidC1.count(candidate)) {
            _c1 = candidate;
            return true;
        }
    }

    // Special: 'q' + 'u' → qu (C1), not q + u(vowel)
    if (_c1 == L"q" && c == L'u') {
        _c1 = L"qu";
        return true;
    }

    return false;
}

bool TelexEngine::TryAddVowel(wchar_t c) {
    // Check for vowel doubling transitions: aa→â, ee→ê, oo→ô
    if (!_v.empty()) {
        wchar_t lastV = _v.back();
        for (const auto& vt : TelexData::VowelTransitions) {
            if (vt.trigger == c && lastV == vt.from) {
                _v.back() = vt.to;
                return true;
            }
        }
        // Check for di-vowel transitions: ie→iê, uo→uô
        for (const auto& dvt : TelexData::DiVowelTransitions) {
            if (dvt.trigger == c && lastV == dvt.first) {
                _v += dvt.result;
                return true;
            }
        }
    }

    // Try adding vowel to nucleus
    std::wstring candidate = _v + c;

    // Check if this vowel combination could be valid
    // We check: exact match, prefix of a valid vowel, or W-transformable to valid
    bool valid = false;

    // Direct match or prefix
    for (const auto& vi : TelexData::ValidVowels) {
        if (vi.vowel == candidate ||
            (vi.vowel.size() > candidate.size() && vi.vowel.substr(0, candidate.size()) == candidate)) {
            valid = true;
            break;
        }
    }

    // Also check if candidate can be W-transformed into something valid
    if (!valid) {
        for (const auto& wt : TelexData::WTransitions) {
            if (wt.from == candidate) {
                valid = true;  // can be transformed by pressing 'w'
                break;
            }
        }
    }

    if (valid || VowelMap().count(candidate)) {
        _v = candidate;
        return true;
    }

    return false;
}

bool TelexEngine::TryAddC2(wchar_t c) {
    if (_v.empty()) return false;

    std::wstring candidate = _c2 + c;

    // Check if this is a valid C2
    if (TelexData::ValidC2.count(candidate)) {
        _c2 = candidate;
        return true;
    }

    // Some keys that look like C2 might actually be tone keys
    // e.g., 'n' after a vowel could be C2 or just consonant
    // We've already classified tone keys before reaching here, so this is C2

    return false;
}

bool TelexEngine::TryAddTone(wchar_t c) {
    if (_v.empty()) return false;

    Tones newTone = TelexData::GetTone(c);

    // If same tone pressed again, undo it (toggle)
    if (_t == newTone && newTone != Tones::Z) {
        _t = Tones::Z;
    } else {
        _t = newTone;
    }
    return true;
}

bool TelexEngine::TryAddW(wchar_t /*c*/) {

    // If no vowel yet and no C1 or C1 is valid before ư
    // standalone 'w' → ư
    if (_v.empty()) {
        // w as standalone vowel ư
        _v = L"\x01b0"; // ư
        return true;
    }

    // Try W transitions on existing vowels
    for (const auto& wt : TelexData::WTransitions) {
        if (_v == wt.from) {
            _v = wt.to;
            return true;
        }
    }

    // If vowel already has W applied, undo it
    for (const auto& wt : TelexData::WTransitions) {
        if (_v == wt.to) {
            _v = wt.from;
            return true;
        }
    }

    return false;
}

int TelexEngine::GetTonePosition() const {
    auto it = VowelMap().find(_v);
    if (it != VowelMap().end()) {
        int pos = it->second.tonePos;
        // Apply oa_uy_tone1 config for specific vowel pairs
        if (_config.oa_uy_tone1) {
            // New style: hoà, uỳ → tone on second vowel
            // This is already the default in our table
        } else {
            // Old style: hòa, ùy → tone on first vowel
            if (_v == L"oa" || _v == L"oe" || _v == L"uy") {
                pos = 0;
            }
        }
        return pos;
    }

    // Fallback: tone on first vowel
    if (_v.size() == 1) return 0;
    if (_v.size() == 2) return 1;
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
    // Apply case tracking: map cases from key input to output chars
    // Simple approach: uppercase the corresponding output positions
    // The case vector tracks input positions, we need to map to output

    // Build mapping: which output chars came from which input positions
    // For simplicity: if C1 was uppercase, uppercase C1 in output, etc.

    if (result.empty() || _cases.empty()) return;

    // Track how many chars each component contributes
    size_t c1Len = _c1.size();
    size_t vLen = _v.size();
    // c2Len is the rest

    size_t inputIdx = 0;
    size_t outputIdx = 0;

    // Apply cases to C1 portion
    for (size_t i = 0; i < c1Len && outputIdx < result.size() && inputIdx < _cases.size(); i++, outputIdx++, inputIdx++) {
        if (_cases[inputIdx]) {
            result[outputIdx] = (wchar_t)towupper(result[outputIdx]);
        }
    }

    // Handle 'dd' → đ (two input chars, one output char)
    if (_hasD && inputIdx < _cases.size()) {
        inputIdx++; // skip second 'd' input
    }

    // Apply cases to vowel portion
    for (size_t i = 0; i < vLen && outputIdx < result.size() && inputIdx < _cases.size(); i++, outputIdx++, inputIdx++) {
        if (_cases[inputIdx]) {
            result[outputIdx] = (wchar_t)towupper(result[outputIdx]);
        }
    }

    // Skip tone key in input (if any)
    if (_t != Tones::Z && inputIdx < _cases.size()) {
        inputIdx++;
    }

    // Apply cases to C2 portion
    for (; outputIdx < result.size() && inputIdx < _cases.size(); outputIdx++, inputIdx++) {
        if (_cases[inputIdx]) {
            result[outputIdx] = (wchar_t)towupper(result[outputIdx]);
        }
    }
}

TelexStates TelexEngine::Commit() {
    if (_state == TelexStates::Invalid || _state == TelexStates::CommittedInvalid) {
        _result = _keyBuffer;
        _state = TelexStates::CommittedInvalid;
        return _state;
    }

    if (_keyBuffer.empty()) {
        _result.clear();
        _state = TelexStates::Committed;
        return _state;
    }

    // Validate the word structure
    if (!_c1.empty() && !TelexData::ValidC1.count(_c1)) {
        _result = _keyBuffer;
        _state = TelexStates::CommittedInvalid;
        return _state;
    }

    // If we have vowels, validate them
    if (!_v.empty() && !VowelMap().count(_v)) {
        _result = _keyBuffer;
        _state = TelexStates::CommittedInvalid;
        return _state;
    }

    if (!_c2.empty() && !TelexData::ValidC2.count(_c2)) {
        _result = _keyBuffer;
        _state = TelexStates::CommittedInvalid;
        return _state;
    }

    // Check tone restriction: restricted C2 only allows S or J tones
    if (!_c2.empty() && TelexData::RestrictedC2.count(_c2)) {
        if (_t != Tones::Z && _t != Tones::S && _t != Tones::J) {
            _result = _keyBuffer;
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
    return _state;
}

TelexStates TelexEngine::Cancel() {
    _result = _keyBuffer;
    _state = TelexStates::CommittedInvalid;
    return _state;
}

TelexStates TelexEngine::Backspace() {
    if (_keyBuffer.empty()) {
        return _state;
    }

    // Remove last character and replay
    _keyBuffer.pop_back();
    if (_cases.size() > 0) {
        _cases.pop_back();
    }

    Replay();
    return _state;
}

void TelexEngine::Replay() {
    std::wstring saved = _keyBuffer;
    auto savedCases = _cases;

    // Reset engine state but keep saved buffer
    _state = TelexStates::Valid;
    _keyBuffer.clear();
    _c1.clear();
    _v.clear();
    _c2.clear();
    _t = Tones::Z;
    _cases.clear();
    _hasD = false;
    _result.clear();

    // Replay each character
    for (size_t i = 0; i < saved.size(); i++) {
        wchar_t c = saved[i];
        // Restore original case
        if (i < savedCases.size() && savedCases[i]) {
            c = (wchar_t)towupper(c);
        }
        PushChar(c);
    }
}

std::wstring TelexEngine::Retrieve() const {
    if (_state == TelexStates::Committed || _state == TelexStates::CommittedInvalid) {
        return _result;
    }
    return _keyBuffer;
}

std::wstring TelexEngine::Peek() const {
    if (_v.empty() && _c1.empty()) {
        return _keyBuffer;
    }

    // Preview: show current composition without finalizing
    std::wstring preview = _c1 + _v + _c2;

    // Apply tone preview if we have vowels
    if (!_v.empty() && _t != Tones::Z) {
        int pos = GetTonePosition();
        std::wstring tonedV = ApplyTone(_v, _t, pos);
        preview = _c1 + tonedV + _c2;
    }

    return preview;
}

} // namespace KType
