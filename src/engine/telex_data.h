#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <array>

namespace KType {
namespace TelexData {

// Tone table: base vowel -> 6 variants [Z, S, F, R, X, J]
// Z=no tone, S=sắc, F=huyền, R=hỏi, X=ngã, J=nặng
struct ToneRow {
    wchar_t base;
    wchar_t toned[6]; // Z, S, F, R, X, J
};

inline const ToneRow ToneTable[] = {
    { L'a', { L'a', L'\x00e1', L'\x00e0', L'\x1ea3', L'\x00e3', L'\x1ea1' } },  // a á à ả ã ạ
    { L'e', { L'e', L'\x00e9', L'\x00e8', L'\x1ebb', L'\x1ebd', L'\x1eb9' } },  // e é è ẻ ẽ ẹ
    { L'i', { L'i', L'\x00ed', L'\x00ec', L'\x1ec9', L'\x0129', L'\x1ecb' } },  // i í ì ỉ ĩ ị
    { L'o', { L'o', L'\x00f3', L'\x00f2', L'\x1ecf', L'\x00f5', L'\x1ecd' } },  // o ó ò ỏ õ ọ
    { L'u', { L'u', L'\x00fa', L'\x00f9', L'\x1ee7', L'\x0169', L'\x1ee5' } },  // u ú ù ủ ũ ụ
    { L'y', { L'y', L'\x00fd', L'\x1ef3', L'\x1ef7', L'\x1ef9', L'\x1ef5' } },  // y ý ỳ ỷ ỹ ỵ
    { L'\x0103', { L'\x0103', L'\x1eaf', L'\x1eb1', L'\x1eb3', L'\x1eb5', L'\x1eb7' } },  // ă ắ ằ ẳ ẵ ặ
    { L'\x00e2', { L'\x00e2', L'\x1ea5', L'\x1ea7', L'\x1ea9', L'\x1eab', L'\x1ead' } },  // â ấ ầ ẩ ẫ ậ
    { L'\x00ea', { L'\x00ea', L'\x1ebf', L'\x1ec1', L'\x1ec3', L'\x1ec5', L'\x1ec7' } },  // ê ế ề ể ễ ệ
    { L'\x00f4', { L'\x00f4', L'\x1ed1', L'\x1ed3', L'\x1ed5', L'\x1ed7', L'\x1ed9' } },  // ô ố ồ ổ ỗ ộ
    { L'\x01a1', { L'\x01a1', L'\x1edb', L'\x1edd', L'\x1edf', L'\x1ee1', L'\x1ee3' } },  // ơ ớ ờ ở ỡ ợ
    { L'\x01b0', { L'\x01b0', L'\x1ee9', L'\x1eeb', L'\x1eed', L'\x1eef', L'\x1ef1' } },  // ư ứ ừ ử ữ ự
};

// Map base vowel char to its ToneTable index
inline std::unordered_map<wchar_t, int> MakeToneMap() {
    std::unordered_map<wchar_t, int> m;
    for (int i = 0; i < (int)std::size(ToneTable); i++) {
        m[ToneTable[i].base] = i;
    }
    return m;
}

// W-key transitions: what vowel(s) transform into when w is pressed
// e.g., "o" → "ơ", "u" → "ư", "a" → "ă"
struct WTransition {
    std::wstring from;
    std::wstring to;
};

inline const WTransition WTransitions[] = {
    { L"o",   L"\x01a1" },                  // o → ơ
    { L"u",   L"\x01b0" },                  // u → ư
    { L"a",   L"\x0103" },                  // a → ă
    { L"uo",  L"\x01b0\x01a1" },            // uo → ươ
    { L"\x01b0o", L"\x01b0\x01a1" },        // ưo → ươ (when ư already typed via uw)
    { L"oa",  L"o\x0103" },                 // oa → oă
    { L"ua",  L"u\x0103" },                 // ua → uă
};

// A-key transitions (doubling vowel adds circumflex): a→â, e→ê, o→ô
struct VowelTransition {
    wchar_t trigger; // the key pressed
    wchar_t from;    // base vowel being modified
    wchar_t to;      // result
};

inline const VowelTransition VowelTransitions[] = {
    { L'a', L'a', L'\x00e2' },  // aa → â
    { L'e', L'e', L'\x00ea' },  // ee → ê
    { L'o', L'o', L'\x00f4' },  // oo → ô
};

// Two-char vowel transitions: "ie" → "iê", "uo" → "uô"
// These transform the second vowel when typed after specific first vowels
struct DiVowelTransition {
    wchar_t first;   // preceding vowel
    wchar_t trigger; // key pressed
    wchar_t result;  // what trigger becomes
};

inline const DiVowelTransition DiVowelTransitions[] = {
    { L'i', L'e', L'\x00ea' },  // ie → iê
    { L'y', L'e', L'\x00ea' },  // ye → yê
    { L'u', L'o', L'\x00f4' },  // uo → uô
};

// Valid onset consonants (C1)
inline const std::unordered_set<std::wstring> ValidC1 = {
    L"", L"b", L"c", L"ch", L"d", L"g", L"gh", L"gi",
    L"h", L"k", L"kh", L"l", L"m", L"n", L"ng", L"ngh",
    L"nh", L"p", L"ph", L"q", L"qu", L"r", L"s", L"t", L"th",
    L"tr", L"v", L"x",
    L"\x0111",  // đ
};

// Valid coda consonants (C2)
// Some codas restrict which tones are allowed:
// c, ch, k, p, t → only S(sắc) or J(nặng) tones allowed
inline const std::unordered_set<std::wstring> ValidC2 = {
    L"", L"c", L"ch", L"k", L"m", L"n", L"ng", L"nh", L"p", L"t",
};

// C2 that restrict tones to only S or J
inline const std::unordered_set<std::wstring> RestrictedC2 = {
    L"c", L"ch", L"k", L"p", L"t",
};

// Valid vowel nuclei with tone position info
struct VowelInfo {
    std::wstring vowel;
    int tonePos;        // which char in vowel gets the tone (0-based), -1 = invalid
    bool requiresC2;    // true = must have a coda
    bool forbidsC2;     // true = cannot have a coda
};

inline const VowelInfo ValidVowels[] = {
    // Single vowels
    { L"a",    0, false, false },
    { L"e",    0, false, false },
    { L"i",    0, false, false },
    { L"o",    0, false, false },
    { L"u",    0, false, false },
    { L"y",    0, false, false },
    { L"\x0103", 0, false, false },  // ă
    { L"\x00e2", 0, false, false },  // â
    { L"\x00ea", 0, false, false },  // ê
    { L"\x00f4", 0, false, false },  // ô
    { L"\x01a1", 0, false, false },  // ơ
    { L"\x01b0", 0, false, false },  // ư

    // Two-vowel combinations
    { L"ai",   0, false, true  },
    { L"ao",   0, false, true  },
    { L"au",   0, false, true  },
    { L"ay",   0, false, true  },
    { L"\x00e2y", 0, false, true  },  // ây
    { L"\x00e2u", 0, false, true  },  // âu
    { L"eo",   0, false, true  },
    { L"\x00eau", 0, false, true  },  // êu
    { L"ia",   0, false, true  },
    { L"iu",   0, false, true  },
    { L"oa",   1, false, false },     // oa: tone on 'a' (new style handled separately)
    { L"oe",   1, false, false },
    { L"oi",   0, false, true  },
    { L"\x00f4i", 0, false, true  },  // ôi
    { L"\x01a1i", 0, false, true  },  // ơi
    { L"ua",   1, false, false },
    { L"ue",   1, false, false },
    { L"ui",   0, false, true  },
    { L"uy",   1, false, false },
    { L"\x01b0\x01a1", 1, false, false },  // ươ: tone on ơ, needs C2
    { L"\x01b0i", 0, false, true  },  // ưi
    { L"\x01b0u", 0, false, true  },  // ưu

    // i + ê combination
    { L"i\x00ea", 1, false, false },  // iê: needs C2
    { L"y\x00ea", 1, false, false },  // yê: needs C2

    // u + ô combination
    { L"u\x00f4", 1, false, false },  // uô: needs C2

    // Three-vowel combinations
    { L"o\x0103", 1, false, false },  // oă
    { L"oai",  1, false, true  },
    { L"oay",  1, false, true  },
    { L"u\x00e2", 1, false, false },  // uâ: needs C2
    { L"u\x00e2y", 1, false, true },  // uây
    { L"uai",  1, false, true  },
    { L"uay",  1, false, true  },
    { L"u\x00ea", 1, false, false },  // uê
    { L"uoi",  1, false, true  },
    { L"u\x00f4i", 1, false, true },  // uôi
    { L"\x01b0\x01a1i", 1, false, true  },  // ươi
    { L"\x01b0\x01a1u", 1, false, true  },  // ươu
    { L"i\x00eau", 1, false, true  },  // iêu
    { L"y\x00eau", 1, false, true  },  // yêu
};

// Map vowel string to its info for quick lookup
inline std::unordered_map<std::wstring, VowelInfo> MakeVowelMap() {
    std::unordered_map<std::wstring, VowelInfo> m;
    for (const auto& v : ValidVowels) {
        m[v.vowel] = v;
    }
    return m;
}

// Characters that can continue a C1 consonant cluster
// e.g., 'h' after 'c','g','k','n','p','t' or 'r' after 't'
inline bool CanContinueC1(const std::wstring& c1, wchar_t next) {
    if (next == L'h') {
        return c1 == L"c" || c1 == L"g" || c1 == L"k" || c1 == L"n"
            || c1 == L"p" || c1 == L"t" || c1 == L"ng";
    }
    if (next == L'g') {
        return c1 == L"n";  // ng
    }
    if (next == L'r') {
        return c1 == L"t";
    }
    if (next == L'i') {
        return c1 == L"g";  // gi
    }
    return false;
}

// Valid base vowels (lowercase)
inline bool IsVowel(wchar_t c) {
    return c == L'a' || c == L'e' || c == L'i' || c == L'o' || c == L'u' || c == L'y';
}

// Telex tone keys
inline Tones GetTone(wchar_t c) {
    switch (c) {
    case L's': return Tones::S;
    case L'f': return Tones::F;
    case L'r': return Tones::R;
    case L'x': return Tones::X;
    case L'j': return Tones::J;
    case L'z': return Tones::Z;
    default:   return Tones::Z;
    }
}

inline bool IsToneKey(wchar_t c) {
    return c == L's' || c == L'f' || c == L'r' || c == L'x' || c == L'j' || c == L'z';
}

inline bool IsConsonant(wchar_t c) {
    switch (c) {
    case L'b': case L'c': case L'd': case L'g': case L'h':
    case L'k': case L'l': case L'm': case L'n': case L'p':
    case L'q': case L'r': case L's': case L't': case L'v':
    case L'x':
        return true;
    default:
        return false;
    }
}

} // namespace TelexData
} // namespace KType
