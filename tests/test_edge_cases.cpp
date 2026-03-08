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
    push(e, "viejt");
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
    push(e, "viejt");
    e.Cancel();
    ASSERT_WSTR_EQ(e.Retrieve(), L"viejt");
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
    push(e, "viej");
    std::wstring preview = e.Peek();
    // iê with nặng = iệ, so preview = viê with tone applied
    ASSERT_WSTR_EQ(preview, L"vi\x1ec7");
    return true;
}

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
    RUN_TEST(test_gi_a);
    RUN_TEST(test_gi_tone);
    printf("\n");
}
