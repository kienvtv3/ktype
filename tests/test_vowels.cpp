#include "test_helper.h"

// Circumflex vowels (doubling)
bool test_aa() { ASSERT_WSTR_EQ(commit("aa"), L"\x00e2"); return true; }   // â standalone letter
bool test_ee() { ASSERT_WSTR_EQ(commit("ee"), L"\x00ea"); return true; }   // ê
bool test_oo() { ASSERT_WSTR_EQ(commit("oo"), L"\x00f4"); return true; }   // ô

// W transitions
bool test_aw() { ASSERT_WSTR_EQ(commit("aw"), L"\x0103"); return true; }   // ă standalone letter
bool test_ow() { ASSERT_WSTR_EQ(commit("ow"), L"\x01a1"); return true; }   // ơ
bool test_uw() { ASSERT_WSTR_EQ(commit("uw"), L"\x01b0"); return true; }   // ư

// Di-vowel transitions (doubling triggers circumflex)
bool test_ie() { ASSERT_WSTR_EQ(commit("iee"), L"iee"); return true; }       // iê requires C2 → invalid
bool test_ye() { ASSERT_WSTR_EQ(commit("yee"), L"yee"); return true; }       // yê requires C2 → invalid
bool test_uo() { ASSERT_WSTR_EQ(commit("uoo"), L"uoo"); return true; }       // uô requires C2 → invalid

// ươ via uw + ow
bool test_uwow() { ASSERT_WSTR_EQ(commit("uwow"), L"uwow"); return true; }  // ươ requires C2 → invalid

// Standalone W
bool test_w_standalone() { ASSERT_WSTR_EQ(commit("w"), L"\x01b0"); return true; }  // ư

// W InvalidateAndPopBack (VietType: 2nd W → pop + invalidate, no reversal/cycling)
bool test_oww() { ASSERT_WSTR_EQ(commit("oww"), L"ow"); return true; }     // oww → raw "ow"
bool test_uww() { ASSERT_WSTR_EQ(commit("uww"), L"uw"); return true; }     // uww → raw "uw"
bool test_aww() { ASSERT_WSTR_EQ(commit("aww"), L"aw"); return true; }     // aww → raw "aw"
bool test_OWW() { ASSERT_WSTR_EQ(commit("OWW"), L"OW"); return true; }     // OWW → raw "OW" (VietType test)

// Leading W undo: ww → raw "w" (not "u")
bool test_ww_raw() { ASSERT_WSTR_EQ(commit("ww"), L"w"); return true; }

// oa, ua W transitions
bool test_oaw() { ASSERT_WSTR_EQ(commit("oaw"), L"oaw"); return true; }      // oă requires C2 → invalid bare

// Vowel transition undo (3rd same char → pop 1 + invalidate, VietType behavior)
bool test_aaa() { ASSERT_WSTR_EQ(commit("aaa"), L"aa"); return true; }       // aaa → raw "aa"
bool test_eee() { ASSERT_WSTR_EQ(commit("eee"), L"ee"); return true; }       // eee → raw "ee"
bool test_ooo() { ASSERT_WSTR_EQ(commit("ooo"), L"oo"); return true; }      // ooo → DoubleUndo, oo MustC2 fail → raw "oo" (3rd 'o' filtered)
bool test_AAA() { ASSERT_WSTR_EQ(commit("AAA"), L"AA"); return true; }       // AAA → raw "AA" (VietType test)
bool test_dataa() { ASSERT_WSTR_EQ(commit("dataa"), L"data"); return true; } // dataa → undo → raw "data"

// (VietType double-key tests moved to test_viettype.cpp)

void run_vowel_tests() {
    printf("Vowel tests:\n");
    RUN_TEST(test_aa);
    RUN_TEST(test_ee);
    RUN_TEST(test_oo);
    RUN_TEST(test_aw);
    RUN_TEST(test_ow);
    RUN_TEST(test_uw);
    RUN_TEST(test_ie);
    RUN_TEST(test_ye);
    RUN_TEST(test_uo);
    RUN_TEST(test_uwow);
    RUN_TEST(test_w_standalone);
    RUN_TEST(test_oww);
    RUN_TEST(test_uww);
    RUN_TEST(test_aww);
    RUN_TEST(test_OWW);
    RUN_TEST(test_ww_raw);
    RUN_TEST(test_oaw);
    RUN_TEST(test_aaa);
    RUN_TEST(test_eee);
    RUN_TEST(test_ooo);
    RUN_TEST(test_AAA);
    RUN_TEST(test_dataa);
    // (VietType double-key tests in test_viettype.cpp)
    printf("\n");
}
