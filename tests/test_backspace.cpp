#include "test_helper.h"

// VietType-model backspace: removes last OUTPUT char, not last keystroke
// "á" (1 output char) → backspace → empty
// "đồng" (4 output chars) → backspace → "đồn" (3 chars)

bool test_bs_remove_tone() {
    TelexEngine e;
    push(e, "as");   // á (1 output char)
    e.Backspace();   // removes 'á' entirely
    e.Commit();
    ASSERT_WSTR_EQ(e.Retrieve(), L"");
    return true;
}

bool test_bs_to_empty() {
    TelexEngine e;
    push(e, "a");
    e.Backspace();
    e.Commit();
    ASSERT_WSTR_EQ(e.Retrieve(), L"");
    return true;
}

bool test_bs_after_circumflex() {
    TelexEngine e;
    push(e, "aa");   // â (1 output char: transition consumed 2nd 'a')
    e.Backspace();   // removes 'â' entirely
    e.Commit();
    ASSERT_WSTR_EQ(e.Retrieve(), L"");
    return true;
}

bool test_bs_after_w() {
    TelexEngine e;
    push(e, "ow");   // ơ (1 output char)
    e.Backspace();   // removes 'ơ' entirely
    e.Commit();
    ASSERT_WSTR_EQ(e.Retrieve(), L"");
    return true;
}

bool test_bs_after_dd() {
    TelexEngine e;
    push(e, "dd");   // đ (1 output char)
    e.Backspace();   // removes 'đ' entirely
    e.Commit();
    ASSERT_WSTR_EQ(e.Retrieve(), L"");
    return true;
}

bool test_bs_mid_word() {
    TelexEngine e;
    push(e, "vieejt");  // việt
    e.Backspace();      // remove 't' → việ
    e.Commit();
    ASSERT_WSTR_EQ(e.Retrieve(), L"vieej"); // iê requires C2 → invalid (raw)
    return true;
}

bool test_bs_multiple() {
    TelexEngine e;
    push(e, "abc");
    e.Backspace();
    e.Backspace();
    e.Commit();
    ASSERT_WSTR_EQ(e.Retrieve(), L"a");
    return true;
}

bool test_bs_on_empty() {
    TelexEngine e;
    e.Backspace(); // should not crash
    e.Commit();
    ASSERT_WSTR_EQ(e.Retrieve(), L"");
    return true;
}

void run_backspace_tests() {
    printf("Backspace tests:\n");
    RUN_TEST(test_bs_remove_tone);
    RUN_TEST(test_bs_to_empty);
    RUN_TEST(test_bs_after_circumflex);
    RUN_TEST(test_bs_after_w);
    RUN_TEST(test_bs_after_dd);
    RUN_TEST(test_bs_mid_word);
    RUN_TEST(test_bs_multiple);
    RUN_TEST(test_bs_on_empty);
    printf("\n");
}
