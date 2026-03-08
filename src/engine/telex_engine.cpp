#include "telex.h"
#include "telex_data.h"
#include "wordlist.h"
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
    _hasW = false;
    _leadingW = false;
    _result.clear();
    _vConsumedKeys = 0;
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

TelexStates TelexEngine::PushChar(wchar_t c) {
    if (_state != TelexStates::Valid && _state != TelexStates::Invalid) {
        return _state;
    }

    // Max syllable length (VietType: MaxLength = 10)
    if (_state == TelexStates::Valid && _keyBuffer.size() >= 10) {
        _state = TelexStates::Invalid;
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
        // Allow C2 after "gi" (where 'i' is stored in C1 but acts as vowel)
        if (_v.empty() && _c1 != L"gi") {
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
    // First 'd': only before any vowels/C2
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

    // Special: 'q' + 'u' -> qu
    if (_c1 == L"q" && c == L'u') {
        _c1 = L"qu";
        return true;
    }

    return false;
}

bool TelexEngine::TryAddVowel(wchar_t c) {
    std::wstring candidate = _v + c;

    // 1. Check if candidate matches a vowel transition (apply it)
    //    Transitions are allowed even after C2 (e.g., "nene" → n+e+n+e → "ee"→"ê" → "nên")
    for (const auto& t : TelexData::VowelTransitions) {
        if (t.from == candidate) {
            _vConsumedKeys += (int)(candidate.size() - t.to.size());
            _v = t.to;
            return true;
        }
    }

    // 2. If C2 is already present and no transition matched, reject
    if (!_c2.empty()) {
        return false;
    }

    // 3. Check if candidate is a prefix of a transition (keep accumulating)
    for (const auto& t : TelexData::VowelTransitions) {
        if (t.from.size() > candidate.size() &&
            t.from.substr(0, candidate.size()) == candidate) {
            _v = candidate;
            return true;
        }
    }

    // 4. Check if candidate is a valid vowel
    if (VowelMap().count(candidate)) {
        _v = candidate;
        return true;
    }

    // 5. Check if candidate is a prefix of a valid vowel
    for (const auto& vi : TelexData::ValidVowels) {
        if (vi.vowel.size() > candidate.size() &&
            vi.vowel.substr(0, candidate.size()) == candidate) {
            _v = candidate;
            return true;
        }
    }

    // 6. Check if candidate can be W-transformed into something valid
    for (const auto& wt : TelexData::WTransitions) {
        if (wt.from == candidate) {
            _v = candidate;
            return true;
        }
    }
    for (const auto& wt : TelexData::WATransitions) {
        if (wt.from == candidate) {
            _v = candidate;
            return true;
        }
    }

    return false;
}

bool TelexEngine::TryAddC2(wchar_t c) {
    if (_v.empty() && _c1 != L"gi") return false;

    std::wstring candidate = _c2 + c;

    if (TelexData::ValidC2.count(candidate)) {
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
        return true;
    }

    return false;
}

bool TelexEngine::TryAddTone(wchar_t c) {
    if (_v.empty() && _c1 != L"gi") return false;

    Tones newTone = TelexData::GetTone(c);

    // Same tone pressed again → undo: pop duplicate, invalidate (VietType behavior)
    // e.g. "tess" → raw "tes", "tesst" → raw "test"
    if (_t == newTone && newTone != Tones::Z) {
        // Remove the duplicate char that PushChar already added to _keyBuffer
        _keyBuffer.pop_back();
        _cases.pop_back();
        _state = TelexStates::Invalid;
        return true;  // handled (don't let PushChar double-invalidate)
    }

    _t = newTone;
    return true;
}

bool TelexEngine::TryAddW(wchar_t /*c*/) {
    // Leading W (empty vowel): only when C1 is also empty (default Telex has no LeadingW)
    if (_v.empty()) {
        if (_c1.empty()) {
            _v = L"\x01b0"; // u-horn
            _hasW = true;
            _leadingW = true;
            return true;
        }
        return false;  // nw, tw etc. → invalid in default Telex
    }

    bool isQu = (_c1 == L"qu");

    // 1. Try W transitions (full or restricted based on C1)
    const auto* wBegin = isQu ? std::begin(TelexData::WTransitionsQ) : std::begin(TelexData::WTransitions);
    const auto* wEnd = isQu ? std::end(TelexData::WTransitionsQ) : std::end(TelexData::WTransitions);
    for (auto it = wBegin; it != wEnd; ++it) {
        if (_v == it->from) {
            _v = it->to;
            _hasW = true;
            return true;
        }
    }

    // 2. Try WA transitions (restricted set for "qu", full for others)
    const auto* waBegin = isQu ? std::begin(TelexData::WATransitionsQ) : std::begin(TelexData::WATransitions);
    const auto* waEnd = isQu ? std::end(TelexData::WATransitionsQ) : std::end(TelexData::WATransitions);
    for (auto it = waBegin; it != waEnd; ++it) {
        if (_v == it->from) {
            _v = it->to;
            _hasW = true;
            return true;
        }
    }

    // 3. Undo W (check against the same tables used for forward)
    //    Leading W (standalone w→ư): pop duplicate + invalidate (like tone undo)
    //    Regular W (ow→ơ): reverse to base vowel
    for (auto it = wBegin; it != wEnd; ++it) {
        if (_v == it->to && it->from != it->to) {  // skip self-transitions
            if (_leadingW) {
                // Leading W undo: "ww" → raw "w" (no base vowel to revert to)
                _keyBuffer.pop_back();
                _cases.pop_back();
                _state = TelexStates::Invalid;
                return true;
            }
            _v = it->from;
            _hasW = false;
            return true;
        }
    }
    for (auto it = waBegin; it != waEnd; ++it) {
        if (_v == it->to) {
            _v = it->from;
            _hasW = false;
            return true;
        }
    }

    return false;
}

int TelexEngine::GetTonePosition() const {
    // Special case: "gi" with empty _v -> tone goes on 'i' (handled in Commit)
    auto it = VowelMap().find(_v);
    if (it != VowelMap().end()) {
        int pos = it->second.tonePos;
        // Apply oa_uy_tone1 config
        if (!_config.oa_uy_tone1) {
            // Old style: hoa -> tone on first vowel
            if (_v == L"oa" || _v == L"oe" || _v == L"uy") {
                pos = 0;
            }
        }
        return pos;
    }

    // Fallback for raw vowel forms (e.g., "uo", "ie" during composition)
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
    if (result.empty() || _cases.empty()) return;

    size_t c1Len = _c1.size();
    size_t vLen = _v.size();

    size_t inputIdx = 0;
    size_t outputIdx = 0;

    // Apply cases to C1 portion
    for (size_t i = 0; i < c1Len && outputIdx < result.size() && inputIdx < _cases.size(); i++, outputIdx++, inputIdx++) {
        if (_cases[inputIdx]) {
            result[outputIdx] = TelexData::VnToUpper(result[outputIdx]);
        }
    }

    // Handle 'dd' -> d-stroke (two input chars, one output char)
    if (_hasD && inputIdx < _cases.size()) {
        inputIdx++; // skip second 'd' input
    }

    // Apply cases to vowel portion
    for (size_t i = 0; i < vLen && outputIdx < result.size() && inputIdx < _cases.size(); i++, outputIdx++, inputIdx++) {
        if (_cases[inputIdx]) {
            result[outputIdx] = TelexData::VnToUpper(result[outputIdx]);
        }
    }

    // Skip extra consumed vowel input keys (from transitions like "iee" -> "ie^")
    for (int i = 0; i < _vConsumedKeys && inputIdx < _cases.size(); i++) {
        inputIdx++;
    }

    // Skip tone key in input (if any)
    if (_t != Tones::Z && inputIdx < _cases.size()) {
        inputIdx++;
    }

    // Skip W key in input (W doesn't produce a separate output char)
    // We detect this by checking if keyBuffer has 'w' that was consumed by TryAddW
    // For simplicity: just map remaining input to C2
    for (; outputIdx < result.size() && inputIdx < _cases.size(); outputIdx++, inputIdx++) {
        if (_cases[inputIdx]) {
            result[outputIdx] = TelexData::VnToUpper(result[outputIdx]);
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

    // English word optimization: reject known English words
    if (_config.optimize_multilang >= 1) {
        std::wstring lower = _keyBuffer;
        for (auto& ch : lower) ch = (wchar_t)towlower(ch);
        if (WordListEn().count(lower)) {
            _result = _keyBuffer;
            _state = TelexStates::CommittedInvalid;
            return _state;
        }
        if (_config.optimize_multilang >= 2 && WordListEn2().count(lower)) {
            _result = _keyBuffer;
            _state = TelexStates::CommittedInvalid;
            return _state;
        }
    }

    // "gi" fixup: if C1 is "gi" and V is empty, move 'i' from C1 to V
    if (_c1 == L"gi" && _v.empty()) {
        _c1 = L"g";
        _v = L"i";
    }

    // Validate C1
    if (!_c1.empty() && !TelexData::ValidC1.count(_c1)) {
        _result = _keyBuffer;
        _state = TelexStates::CommittedInvalid;
        return _state;
    }

    // Validate vowels
    if (!_v.empty() && !VowelMap().count(_v)) {
        _result = _keyBuffer;
        _state = TelexStates::CommittedInvalid;
        return _state;
    }

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

    // Validate C2
    if (!_c2.empty() && !TelexData::ValidC2.count(_c2)) {
        _result = _keyBuffer;
        _state = TelexStates::CommittedInvalid;
        return _state;
    }

    // Check tone restriction: restricted C2 only allows S or J tones
    // Exception: d-bar (đ) onset bypasses this check (teencode, VietType behavior)
    if (!_c2.empty() && TelexData::RestrictedC2.count(_c2) && _c1 != L"\x0111") {
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

    // Reset engine state
    _state = TelexStates::Valid;
    _keyBuffer.clear();
    _c1.clear();
    _v.clear();
    _c2.clear();
    _t = Tones::Z;
    _cases.clear();
    _hasD = false;
    _hasW = false;
    _leadingW = false;
    _result.clear();
    _vConsumedKeys = 0;

    // Replay each character
    for (size_t i = 0; i < saved.size(); i++) {
        wchar_t c = saved[i];
        // Restore original case
        if (i < savedCases.size() && savedCases[i]) {
            c = TelexData::VnToUpper(c);
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

    // For "gi" with no vowel: show "gi" during composition
    std::wstring previewC1 = _c1;
    std::wstring previewV = _v;

    // Preview: show current composition
    std::wstring preview = previewC1 + previewV + _c2;

    // Apply tone preview if we have vowels
    if (!previewV.empty() && _t != Tones::Z) {
        auto it = VowelMap().find(previewV);
        int pos = 0;
        if (it != VowelMap().end()) {
            pos = it->second.tonePos;
            if (!_config.oa_uy_tone1) {
                if (previewV == L"oa" || previewV == L"oe" || previewV == L"uy") {
                    pos = 0;
                }
            }
        } else if (previewV.size() >= 2) {
            pos = 1;
        }
        std::wstring tonedV = ApplyTone(previewV, _t, pos);
        preview = previewC1 + tonedV + _c2;
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

} // namespace KType
