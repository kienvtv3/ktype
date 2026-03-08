#include "test_helper.h"

// Circumflex vowels (doubling)
bool test_aa() { ASSERT_WSTR_EQ(commit("aa"), L"aa"); return true; }       // â requires C2 → invalid bare
bool test_ee() { ASSERT_WSTR_EQ(commit("ee"), L"\x00ea"); return true; }   // ê
bool test_oo() { ASSERT_WSTR_EQ(commit("oo"), L"\x00f4"); return true; }   // ô

// W transitions
bool test_aw() { ASSERT_WSTR_EQ(commit("aw"), L"aw"); return true; }       // ă requires C2 → invalid bare
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

// W undo (toggle)
bool test_w_undo() { ASSERT_WSTR_EQ(commit("oww"), L"o"); return true; }

// oa, ua W transitions
bool test_oaw() { ASSERT_WSTR_EQ(commit("oaw"), L"oaw"); return true; }      // oă requires C2 → invalid bare

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
    RUN_TEST(test_w_undo);
    RUN_TEST(test_oaw);
    printf("\n");
}
