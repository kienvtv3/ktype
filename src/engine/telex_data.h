#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <array>

namespace KType {
namespace TelexData {

// Tone table: base vowel -> 6 variants [Z, S, F, R, X, J]
// Z=no tone, S=sac, F=huyen, R=hoi, X=nga, J=nang
struct ToneRow {
    wchar_t base;
    wchar_t toned[6]; // Z, S, F, R, X, J
};

inline const ToneRow ToneTable[] = {
    { L'a', { L'a', L'\x00e1', L'\x00e0', L'\x1ea3', L'\x00e3', L'\x1ea1' } },  // a
    { L'e', { L'e', L'\x00e9', L'\x00e8', L'\x1ebb', L'\x1ebd', L'\x1eb9' } },  // e
    { L'i', { L'i', L'\x00ed', L'\x00ec', L'\x1ec9', L'\x0129', L'\x1ecb' } },  // i
    { L'o', { L'o', L'\x00f3', L'\x00f2', L'\x1ecf', L'\x00f5', L'\x1ecd' } },  // o
    { L'u', { L'u', L'\x00fa', L'\x00f9', L'\x1ee7', L'\x0169', L'\x1ee5' } },  // u
    { L'y', { L'y', L'\x00fd', L'\x1ef3', L'\x1ef7', L'\x1ef9', L'\x1ef5' } },  // y
    { L'\x0103', { L'\x0103', L'\x1eaf', L'\x1eb1', L'\x1eb3', L'\x1eb5', L'\x1eb7' } },  // a-breve
    { L'\x00e2', { L'\x00e2', L'\x1ea5', L'\x1ea7', L'\x1ea9', L'\x1eab', L'\x1ead' } },  // a-circumflex
    { L'\x00ea', { L'\x00ea', L'\x1ebf', L'\x1ec1', L'\x1ec3', L'\x1ec5', L'\x1ec7' } },  // e-circumflex
    { L'\x00f4', { L'\x00f4', L'\x1ed1', L'\x1ed3', L'\x1ed5', L'\x1ed7', L'\x1ed9' } },  // o-circumflex
    { L'\x01a1', { L'\x01a1', L'\x1edb', L'\x1edd', L'\x1edf', L'\x1ee1', L'\x1ee3' } },  // o-horn
    { L'\x01b0', { L'\x01b0', L'\x1ee9', L'\x1eeb', L'\x1eed', L'\x1eef', L'\x1ef1' } },  // u-horn
};

// Map base vowel char to its ToneTable index
inline std::unordered_map<wchar_t, int> MakeToneMap() {
    std::unordered_map<wchar_t, int> m;
    for (int i = 0; i < (int)std::size(ToneTable); i++) {
        m[ToneTable[i].base] = i;
    }
    return m;
}

// Vowel transitions: maps accumulated _v string to result.
// VietType-style: NO eager di-vowel transforms. Transitions fire on the
// doubling key (e.g. "iee" -> "ie^", "uoo" -> "uo^").
struct VTransition {
    std::wstring from;
    std::wstring to;
};

inline const VTransition VowelTransitions[] = {
    // Single doubling: aa->a^, ee->e^, oo->o^
    { L"aa",    L"\x00e2" },
    { L"ee",    L"\x00ea" },
    { L"oo",    L"\x00f4" },

    // Delayed di-vowel transitions (the KEY fix)
    { L"iee",   L"i\x00ea" },        // ie+e -> ie^
    { L"yee",   L"y\x00ea" },        // ye+e -> ye^
    { L"uoo",   L"u\x00f4" },        // uo+o -> uo^
    { L"uee",   L"u\x00ea" },        // ue+e -> ue^
    { L"uaa",   L"u\x00e2" },        // ua+a -> ua^
    { L"uyee",  L"uy\x00ea" },       // uye+e -> uye^

    // Alternative input order (doubling after next vowel)
    { L"aua",   L"\x00e2u" },        // au+a -> a^u
    { L"aya",   L"\x00e2y" },        // ay+a -> a^y
    { L"eue",   L"\x00eau" },        // eu+e -> e^u
    { L"oio",   L"\x00f4i" },        // oi+o -> o^i
    { L"uaya",  L"u\x00e2y" },      // uay+a -> ua^y
    { L"uoio",  L"u\x00f4i" },      // uoi+o -> uo^i
    { L"ieue",  L"i\x00eau" },      // ieu+e -> ie^u
    { L"yeue",  L"y\x00eau" },      // yeu+e -> ye^u

    // Reverse: ô+o → oo (undo circumflex, for "xooong" → "xoong")
    { L"\x00f4o", L"oo" },
};

// W-key transitions (horn: adds hook to o/u)
struct WTransition {
    std::wstring from;
    std::wstring to;
};

inline const WTransition WTransitions[] = {
    { L"o",    L"\x01a1" },                      // o -> o-horn
    { L"oi",   L"\x01a1i" },                     // oi -> o-horn i
    { L"u",    L"\x01b0" },                      // u -> u-horn
    { L"ua",   L"\x01b0" L"a" },                 // ua -> u-horn a
    { L"ui",   L"\x01b0i" },                     // ui -> u-horn i
    { L"uo",   L"\x01b0\x01a1" },                // uo -> u-horn o-horn
    { L"uoi",  L"\x01b0\x01a1i" },               // uoi -> u-horn o-horn i
    { L"uou",  L"\x01b0\x01a1u" },               // uou -> u-horn o-horn u
    { L"uu",   L"\x01b0u" },                     // uu -> u-horn u
    { L"\x01b0o", L"\x01b0\x01a1" },             // u-horn o -> u-horn o-horn
    { L"\x01b0\x01a1", L"\x01b0\x01a1" },       // u-horn o-horn -> self (prevent double-w undo)
};

// WA transitions (breve: aw -> a-breve). Tried when W transitions fail.
inline const WTransition WATransitions[] = {
    { L"a",    L"\x0103" },                      // a -> a-breve
    { L"oa",   L"o\x0103" },                     // oa -> o a-breve
    { L"ua",   L"u\x0103" },                     // ua -> u a-breve
};

// W transitions after "qu" (restricted set — VietType: transitions_w_q)
inline const WTransition WTransitionsQ[] = {
    { L"u",    L"\x01b0" },                      // u → u-horn
    { L"uo",   L"u\x01a1" },                     // uo → u o-horn
    { L"uoi",  L"u\x01a1i" },                    // uoi → u o-horn i
};

// WA transitions after "qu" (VietType: transitions_wa_q)
// Note: after "qu", 'u' is in C1, so _v starts with 'a' not 'ua'
inline const WTransition WATransitionsQ[] = {
    { L"a",    L"\x0103" },                       // a → a-breve
};

// Vowel adjustments triggered when C2 is typed (VietType: transitions_wv_c2)
// These adjust the vowel form based on whether a final consonant follows.
inline const WTransition WvC2Transitions[] = {
    { L"u\x01a1",          L"\x01b0\x01a1" },     // uơ → ươ
    { L"\x01b0" L"a",      L"u\x0103" },           // ưa → uă
    { L"\x01b0o",          L"\x01b0\x01a1" },       // ưo → ươ
};

// Valid onset consonants (C1)
inline const std::unordered_set<std::wstring> ValidC1 = {
    L"", L"b", L"c", L"ch", L"d", L"g", L"gh", L"gi",
    L"h", L"k", L"kh", L"l", L"m", L"n", L"ng", L"ngh",
    L"nh", L"p", L"ph", L"q", L"qu", L"r", L"s", L"t", L"th",
    L"tr", L"v", L"x",
    L"\x0111",  // d-stroke
};

// Valid coda consonants (C2)
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
    int tonePos;        // which char in vowel gets the tone (0-based)
    bool requiresC2;    // must have a coda
    bool forbidsC2;     // cannot have a coda
};

inline const VowelInfo ValidVowels[] = {
    // Single vowels
    { L"a",    0, false, false },
    { L"e",    0, false, false },
    { L"i",    0, false, false },
    { L"o",    0, false, false },
    { L"u",    0, false, false },
    { L"y",    0, false, true  },  // y: forbids C2 (VietType)
    { L"\x0103", 0, true, false },   // a-breve: requires C2
    { L"\x00e2", 0, true, false },   // a-circumflex: requires C2 (VietType)
    { L"\x00ea", 0, false, false },  // e-circumflex
    { L"\x00f4", 0, false, false },  // o-circumflex
    { L"\x01a1", 0, false, false },  // o-horn
    { L"\x01b0", 0, false, false },  // u-horn

    // Two-vowel combinations
    { L"oo",   1, true, false },    // double-o: requires C2, tonePos=1 (VietType)
    { L"ai",   0, false, true  },
    { L"ao",   0, false, true  },
    { L"au",   0, false, true  },
    { L"ay",   0, false, true  },
    { L"\x00e2y", 0, false, true  },  // a-circumflex y
    { L"\x00e2u", 0, false, true  },  // a-circumflex u
    { L"eo",   0, false, true  },
    { L"\x00eau", 0, false, true  },  // e-circumflex u
    { L"ia",   0, false, true  },
    { L"iu",   0, false, true  },
    { L"oa",   1, false, false },
    { L"oe",   1, false, false },
    { L"oi",   0, false, true  },
    { L"\x00f4i", 0, false, true  },  // o-circumflex i
    { L"\x01a1i", 0, false, true  },  // o-horn i
    { L"ua",   0, false, true  },   // ua: tonePos=0 (của not cuả), forbids C2 (VietType)
    { L"ue",   1, false, false },
    { L"ui",   0, false, true  },
    { L"uy",   1, false, false },
    { L"\x01b0\x01a1", 1, true, false },  // u-horn o-horn: requires C2
    { L"\x01b0i", 0, false, true  },     // u-horn i
    { L"\x01b0u", 0, false, true  },     // u-horn u
    { L"\x01b0" L"a", 0, false, true  }, // u-horn a: forbids C2 (VietType)

    // i + e-circumflex combination
    { L"i\x00ea", 1, true, false },  // ie-circumflex: requires C2
    { L"y\x00ea", 1, true, false },  // ye-circumflex: requires C2

    // u + o-circumflex combination
    { L"u\x00f4", 1, true, false },  // uo-circumflex: requires C2

    // u + e-circumflex
    { L"u\x00ea", 1, false, false },

    // Three-vowel combinations
    { L"o\x0103", 1, true, false },   // o a-breve: requires C2 (VietType)
    { L"u\x0103", 1, true, false },   // u a-breve: requires C2 (VietType)
    { L"oai",  1, false, true  },
    { L"oay",  1, false, true  },
    { L"u\x00e2", 1, true, false },   // u a-circumflex: requires C2
    { L"u\x00e2y", 1, false, true },  // u a-circumflex y
    { L"uai",  1, false, true  },
    { L"uay",  1, false, true  },
    { L"uoi",  1, false, true  },
    { L"u\x00f4i", 1, false, true },  // u o-circumflex i
    { L"\x01b0\x01a1i", 1, false, true  },  // u-horn o-horn i
    { L"\x01b0\x01a1u", 1, false, true  },  // u-horn o-horn u
    { L"i\x00eau", 1, false, true  },       // i e-circumflex u
    { L"y\x00eau", 1, false, true  },       // y e-circumflex u
    { L"uy\x00ea", 2, true, false },        // uy e-circumflex: requires C2
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

// Custom Vietnamese uppercase mapping (towupper fails for Vietnamese diacritics)
inline wchar_t VnToUpper(wchar_t c) {
    if (c >= L'a' && c <= L'z') return c - 32;
    switch (c) {
    case L'\x0111': return L'\x0110'; // đ→Đ
    case L'\x0103': return L'\x0102'; // ă→Ă
    case L'\x1eaf': return L'\x1eae'; // ắ→Ắ
    case L'\x1eb1': return L'\x1eb0'; // ằ→Ằ
    case L'\x1eb3': return L'\x1eb2'; // ẳ→Ẳ
    case L'\x1eb5': return L'\x1eb4'; // ẵ→Ẵ
    case L'\x1eb7': return L'\x1eb6'; // ặ→Ặ
    case L'\x00e2': return L'\x00c2'; // â→Â
    case L'\x1ea5': return L'\x1ea4'; // ấ→Ấ
    case L'\x1ea7': return L'\x1ea6'; // ầ→Ầ
    case L'\x1ea9': return L'\x1ea8'; // ẩ→Ẩ
    case L'\x1eab': return L'\x1eaa'; // ẫ→Ẫ
    case L'\x1ead': return L'\x1eac'; // ậ→Ậ
    case L'\x00e1': return L'\x00c1'; // á→Á
    case L'\x00e0': return L'\x00c0'; // à→À
    case L'\x1ea3': return L'\x1ea2'; // ả→Ả
    case L'\x00e3': return L'\x00c3'; // ã→Ã
    case L'\x1ea1': return L'\x1ea0'; // ạ→Ạ
    case L'\x00ea': return L'\x00ca'; // ê→Ê
    case L'\x1ebf': return L'\x1ebe'; // ế→Ế
    case L'\x1ec1': return L'\x1ec0'; // ề→Ề
    case L'\x1ec3': return L'\x1ec2'; // ể→Ể
    case L'\x1ec5': return L'\x1ec4'; // ễ→Ễ
    case L'\x1ec7': return L'\x1ec6'; // ệ→Ệ
    case L'\x00e9': return L'\x00c9'; // é→É
    case L'\x00e8': return L'\x00c8'; // è→È
    case L'\x1ebb': return L'\x1eba'; // ẻ→Ẻ
    case L'\x1ebd': return L'\x1ebc'; // ẽ→Ẽ
    case L'\x1eb9': return L'\x1eb8'; // ẹ→Ẹ
    case L'\x00ed': return L'\x00cd'; // í→Í
    case L'\x00ec': return L'\x00cc'; // ì→Ì
    case L'\x1ec9': return L'\x1ec8'; // ỉ→Ỉ
    case L'\x0129': return L'\x0128'; // ĩ→Ĩ
    case L'\x1ecb': return L'\x1eca'; // ị→Ị
    case L'\x00f4': return L'\x00d4'; // ô→Ô
    case L'\x1ed1': return L'\x1ed0'; // ố→Ố
    case L'\x1ed3': return L'\x1ed2'; // ồ→Ồ
    case L'\x1ed5': return L'\x1ed4'; // ổ→Ổ
    case L'\x1ed7': return L'\x1ed6'; // ỗ→Ỗ
    case L'\x1ed9': return L'\x1ed8'; // ộ→Ộ
    case L'\x01a1': return L'\x01a0'; // ơ→Ơ
    case L'\x1edb': return L'\x1eda'; // ớ→Ớ
    case L'\x1edd': return L'\x1edc'; // ờ→Ờ
    case L'\x1edf': return L'\x1ede'; // ở→Ở
    case L'\x1ee1': return L'\x1ee0'; // ỡ→Ỡ
    case L'\x1ee3': return L'\x1ee2'; // ợ→Ợ
    case L'\x00f3': return L'\x00d3'; // ó→Ó
    case L'\x00f2': return L'\x00d2'; // ò→Ò
    case L'\x1ecf': return L'\x1ece'; // ỏ→Ỏ
    case L'\x00f5': return L'\x00d5'; // õ→Õ
    case L'\x1ecd': return L'\x1ecc'; // ọ→Ọ
    case L'\x01b0': return L'\x01af'; // ư→Ư
    case L'\x1ee9': return L'\x1ee8'; // ứ→Ứ
    case L'\x1eeb': return L'\x1eea'; // ừ→Ừ
    case L'\x1eed': return L'\x1eec'; // ử→Ử
    case L'\x1eef': return L'\x1eee'; // ữ→Ữ
    case L'\x1ef1': return L'\x1ef0'; // ự→Ự
    case L'\x00fa': return L'\x00da'; // ú→Ú
    case L'\x00f9': return L'\x00d9'; // ù→Ù
    case L'\x1ee7': return L'\x1ee6'; // ủ→Ủ
    case L'\x0169': return L'\x0168'; // ũ→Ũ
    case L'\x1ee5': return L'\x1ee4'; // ụ→Ụ
    case L'\x00fd': return L'\x00dd'; // ý→Ý
    case L'\x1ef3': return L'\x1ef2'; // ỳ→Ỳ
    case L'\x1ef7': return L'\x1ef6'; // ỷ→Ỷ
    case L'\x1ef9': return L'\x1ef8'; // ỹ→Ỹ
    case L'\x1ef5': return L'\x1ef4'; // ỵ→Ỵ
    default: return (wchar_t)towupper(c);
    }
}

} // namespace TelexData
} // namespace KType
