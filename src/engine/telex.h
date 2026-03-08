#pragma once

#include <string>
#include <vector>

namespace KType {

enum class TelexStates {
    Valid,              // Can accept more chars, word looks valid so far
    Invalid,            // Word is invalid but can still accept chars
    Committed,          // Valid word finalized with tones applied
    CommittedInvalid,   // Invalid word finalized as raw keys
};

enum class Tones {
    Z = 0,  // No tone
    S = 1,  // Sắc (acute)
    F = 2,  // Huyền (grave)
    R = 3,  // Hỏi (hook above)
    X = 4,  // Ngã (tilde)
    J = 5,  // Nặng (dot below)
};

struct TelexConfig {
    // true = new style: tone on second vowel (hoà, uỳ)
    // false = old style: tone on first vowel (hòa, ùy)
    bool oa_uy_tone1 = true;
    bool accept_separate_dd = true;  // Allow dd→đ even after vowels typed
};

class TelexEngine {
public:
    explicit TelexEngine(const TelexConfig& config = {});

    void Reset();
    TelexStates PushChar(wchar_t c);
    TelexStates Backspace();
    TelexStates Commit();
    TelexStates Cancel();

    TelexStates GetState() const { return _state; }
    std::wstring Retrieve() const;
    std::wstring RetrieveRaw() const { return _keyBuffer; }
    std::wstring Peek() const;
    size_t Count() const { return _keyBuffer.size(); }
    bool IsEmpty() const { return _keyBuffer.empty(); }

    static bool AcceptsChar(wchar_t c);

    void SetConfig(const TelexConfig& config) { _config = config; }
    const TelexConfig& GetConfig() const { return _config; }

private:
    enum class CharType {
        Consonant,      // b, c, d, g, h, k, l, m, n, p, q, r, s, t, v, x
        Vowel,          // a, e, i, o, u, y
        ToneMark,       // s, f, r, x, j, z
        TransitionW,    // w (transforms vowels: o→ơ, u→ư, a→ă)
        DoubleD,        // d (for đ when doubled)
        Invalid,
    };

    CharType Classify(wchar_t c) const;
    bool TryAddC1(wchar_t c);
    bool TryAddVowel(wchar_t c);
    bool TryAddC2(wchar_t c);
    bool TryAddTone(wchar_t c);
    bool TryAddW(wchar_t c);
    bool TryAddD(wchar_t c);

    int GetTonePosition() const;
    std::wstring ApplyTone(const std::wstring& vowel, Tones tone, int pos) const;
    void ApplyCases(std::wstring& result) const;
    void Replay();

    TelexConfig _config;
    TelexStates _state = TelexStates::Valid;
    std::wstring _keyBuffer;
    std::wstring _c1;       // onset consonant(s)
    std::wstring _v;        // vowel nucleus
    std::wstring _c2;       // coda consonant(s)
    Tones _t = Tones::Z;
    std::vector<int> _cases; // 1=upper, 0=lower per output char
    bool _hasD = false;      // whether đ was typed (dd)
    int _vConsumedKeys = 0;  // extra input keys consumed by vowel transitions
    std::wstring _result;    // stored result after Commit
};

} // namespace KType
