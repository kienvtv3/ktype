#include "test_helper.h"

// Basic tone marks
bool test_sac()  { ASSERT_WSTR_EQ(commit("as"), L"\x00e1"); return true; }   // á
bool test_huyen() { ASSERT_WSTR_EQ(commit("af"), L"\x00e0"); return true; }  // à
bool test_hoi()  { ASSERT_WSTR_EQ(commit("ar"), L"\x1ea3"); return true; }   // ả
bool test_nga()  { ASSERT_WSTR_EQ(commit("ax"), L"\x00e3"); return true; }   // ã
bool test_nang() { ASSERT_WSTR_EQ(commit("aj"), L"\x1ea1"); return true; }   // ạ

// Tones on different vowels
bool test_tone_e() { ASSERT_WSTR_EQ(commit("es"), L"\x00e9"); return true; }  // é
bool test_tone_i() { ASSERT_WSTR_EQ(commit("is"), L"\x00ed"); return true; }  // í
bool test_tone_o() { ASSERT_WSTR_EQ(commit("os"), L"\x00f3"); return true; }  // ó
bool test_tone_u() { ASSERT_WSTR_EQ(commit("us"), L"\x00fa"); return true; }  // ú
bool test_tone_y() { ASSERT_WSTR_EQ(commit("ys"), L"\x00fd"); return true; }  // ý

// Tones on modified vowels
bool test_tone_aw() { ASSERT_WSTR_EQ(commit("awns"), L"\x1eafn"); return true; }  // ắn (ă requires C2)
bool test_tone_aa() { ASSERT_WSTR_EQ(commit("aans"), L"\x1ea5n"); return true; }  // ấn (â requires C2)
bool test_tone_ee() { ASSERT_WSTR_EQ(commit("ees"), L"\x1ebf"); return true; }  // ế
bool test_tone_oo() { ASSERT_WSTR_EQ(commit("oos"), L"\x1ed1"); return true; }  // ố
bool test_tone_ow() { ASSERT_WSTR_EQ(commit("ows"), L"\x1edb"); return true; }  // ớ
bool test_tone_uw() { ASSERT_WSTR_EQ(commit("uws"), L"\x1ee9"); return true; }  // ứ

// Tone double-press: same tone twice → invalidate (VietType behavior)
bool test_tone_toggle() { ASSERT_WSTR_EQ(commit("ass"), L"ass"); return true; }

// Tone replacement (different tone replaces)
bool test_tone_replace() { ASSERT_WSTR_EQ(commit("asf"), L"\x00e0"); return true; }  // à

// Tone on two-vowel combo
bool test_tone_ai() { ASSERT_WSTR_EQ(commit("ais"), L"\x00e1i"); return true; }  // ái
bool test_tone_oi() { ASSERT_WSTR_EQ(commit("ois"), L"\x00f3i"); return true; }  // ói
bool test_tone_ao() { ASSERT_WSTR_EQ(commit("aos"), L"\x00e1o"); return true; }  // áo

void run_tone_tests() {
    printf("Tone tests:\n");
    RUN_TEST(test_sac);
    RUN_TEST(test_huyen);
    RUN_TEST(test_hoi);
    RUN_TEST(test_nga);
    RUN_TEST(test_nang);
    RUN_TEST(test_tone_e);
    RUN_TEST(test_tone_i);
    RUN_TEST(test_tone_o);
    RUN_TEST(test_tone_u);
    RUN_TEST(test_tone_y);
    RUN_TEST(test_tone_aw);
    RUN_TEST(test_tone_aa);
    RUN_TEST(test_tone_ee);
    RUN_TEST(test_tone_oo);
    RUN_TEST(test_tone_ow);
    RUN_TEST(test_tone_uw);
    RUN_TEST(test_tone_toggle);
    RUN_TEST(test_tone_replace);
    RUN_TEST(test_tone_ai);
    RUN_TEST(test_tone_oi);
    RUN_TEST(test_tone_ao);
    printf("\n");
}
