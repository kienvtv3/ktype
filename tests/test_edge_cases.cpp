#include "test_helper.h"

// Tone placement: new style (oa_uy_tone1 = true, default)
bool test_tone_pos_hoa_new() {
    TelexConfig cfg;
    cfg.oa_uy_tone1 = true;
    ASSERT_WSTR_EQ(commit("hoaf", cfg), L"ho\x00e0"); // hoà (tone on a)
    return true;
}

// Tone placement: old style (oa_uy_tone1 = false)
bool test_tone_pos_hoa_old() {
    TelexConfig cfg;
    cfg.oa_uy_tone1 = false;
    ASSERT_WSTR_EQ(commit("hoaf", cfg), L"h\x00f2" L"a"); // hòa (tone on o)
    return true;
}

bool test_tone_pos_uy_new() {
    TelexConfig cfg;
    cfg.oa_uy_tone1 = true;
    ASSERT_WSTR_EQ(commit("huys"), L"hu\x00fd"); // huý (tone on y)
    return true;
}

bool test_tone_pos_uy_old() {
    TelexConfig cfg;
    cfg.oa_uy_tone1 = false;
    ASSERT_WSTR_EQ(commit("huys", cfg), L"h\x00fay"); // húy (tone on u)
    return true;
}

// AcceptsChar
bool test_accepts_alpha() {
    TelexEngine e;
    ASSERT_TRUE(e.AcceptsChar(L'a'));
    ASSERT_TRUE(e.AcceptsChar(L'Z'));
    ASSERT_FALSE(e.AcceptsChar(L'1'));
    ASSERT_FALSE(e.AcceptsChar(L' '));
    ASSERT_FALSE(e.AcceptsChar(L'@'));
    return true;
}

// Reset
bool test_reset() {
    TelexEngine e;
    push(e, "vieejt");
    e.Commit();
    ASSERT_WSTR_EQ(e.Retrieve(), L"vi\x1ec7t");
    e.Reset();
    push(e, "nam");
    e.Commit();
    ASSERT_WSTR_EQ(e.Retrieve(), L"nam");
    return true;
}

// Cancel returns raw input
bool test_cancel() {
    TelexEngine e;
    push(e, "vieejt");
    e.Cancel();
    ASSERT_WSTR_EQ(e.Retrieve(), L"vieejt");
    return true;
}

// Empty commit
bool test_empty_commit() {
    TelexEngine e;
    e.Commit();
    ASSERT_WSTR_EQ(e.Retrieve(), L"");
    return true;
}

// Peek (preview without committing)
bool test_peek() {
    TelexEngine e;
    push(e, "vieej");
    std::wstring preview = e.Peek();
    // iê with nặng = iệ, so preview = viê with tone applied
    ASSERT_WSTR_EQ(preview, L"vi\x1ec7");
    return true;
}

// C2Mode: MustC2 — iê requires coda
bool test_ie_no_c2()   { ASSERT_WSTR_EQ(commit("iee"), L"iee"); return true; }          // iê alone → invalid
bool test_ie_with_c2() { ASSERT_WSTR_EQ(commit("ieeng"), L"i\x00eang"); return true; }  // iêng → valid

// C2Mode: NoC2 — ai forbids coda
bool test_ai_no_c2()   { ASSERT_WSTR_EQ(commit("ai"), L"ai"); return true; }             // ai → valid
bool test_ai_with_c2() { ASSERT_WSTR_EQ(commit("ain"), L"ain"); return true; }           // ain → invalid (raw)

// C2Mode: uô requires C2
bool test_uo_no_c2()   { ASSERT_WSTR_EQ(commit("uoo"), L"uoo"); return true; }          // uô alone → invalid
bool test_uo_with_c2() { ASSERT_WSTR_EQ(commit("uoong"), L"u\x00f4ng"); return true; }  // uông → valid

// Preview matches commit when C2 is invalid (no misleading vowel transitions)
bool test_peek_invalid_c2_kobo() {
    TelexEngine e;
    push(e, "kobo");
    ASSERT_WSTR_EQ(e.Peek(), L"kobo");   // not "kôb"
    e.Commit();
    ASSERT_WSTR_EQ(e.Retrieve(), L"kobo");
    return true;
}

bool test_peek_invalid_c2_logo() {
    TelexEngine e;
    push(e, "logo");
    ASSERT_WSTR_EQ(e.Peek(), L"logo");   // not "lôg"
    e.Commit();
    ASSERT_WSTR_EQ(e.Retrieve(), L"logo");
    return true;
}

bool test_peek_invalid_c2_push() {
    TelexEngine e;
    push(e, "push");
    ASSERT_WSTR_EQ(e.Peek(), L"push");   // not "púh"
    e.Commit();
    ASSERT_WSTR_EQ(e.Retrieve(), L"push");
    return true;
}

// Preview matches commit for English words in wordlist
bool test_peek_english_virus() {
    TelexConfig cfg;
    cfg.optimize_multilang = 1;
    TelexEngine e(cfg);
    push(e, "virus");
    ASSERT_WSTR_EQ(e.Peek(), L"virus");  // not "vírú"
    e.Commit();
    ASSERT_WSTR_EQ(e.Retrieve(), L"virus");
    return true;
}

// Max word length: >10 chars → invalid (raw)
bool test_max_length() { ASSERT_WSTR_EQ(commit("abcdefghijk"), L"abcdefghijk"); return true; }

// gi handling: gi is C1, vowel after gi works normally
bool test_gi_a() {
    ASSERT_WSTR_EQ(commit("gia"), L"gia");
    return true;
}

bool test_gi_tone() {
    ASSERT_WSTR_EQ(commit("gias"), L"gi\x00e1"); // giá
    return true;
}

void run_edge_case_tests() {
    printf("Edge case tests:\n");
    RUN_TEST(test_tone_pos_hoa_new);
    RUN_TEST(test_tone_pos_hoa_old);
    RUN_TEST(test_tone_pos_uy_new);
    RUN_TEST(test_tone_pos_uy_old);
    RUN_TEST(test_accepts_alpha);
    RUN_TEST(test_reset);
    RUN_TEST(test_cancel);
    RUN_TEST(test_empty_commit);
    RUN_TEST(test_peek);
    RUN_TEST(test_ie_no_c2);
    RUN_TEST(test_ie_with_c2);
    RUN_TEST(test_ai_no_c2);
    RUN_TEST(test_ai_with_c2);
    RUN_TEST(test_uo_no_c2);
    RUN_TEST(test_uo_with_c2);
    RUN_TEST(test_peek_invalid_c2_kobo);
    RUN_TEST(test_peek_invalid_c2_logo);
    RUN_TEST(test_peek_invalid_c2_push);
    RUN_TEST(test_peek_english_virus);
    RUN_TEST(test_gi_a);
    RUN_TEST(test_gi_tone);
    RUN_TEST(test_max_length);
    printf("\n");
}
