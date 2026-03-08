#include "test_helper.h"

// Backspace removes last character and replays
bool test_bs_remove_tone() {
    TelexEngine e;
    push(e, "as");   // á
    e.Backspace();   // remove 's' → a
    e.Commit();
    ASSERT_WSTR_EQ(e.Retrieve(), L"a");
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
    push(e, "aa");   // â
    e.Backspace();   // remove second 'a' → a
    e.Commit();
    ASSERT_WSTR_EQ(e.Retrieve(), L"a");
    return true;
}

bool test_bs_after_w() {
    TelexEngine e;
    push(e, "ow");   // ơ
    e.Backspace();   // remove 'w' → o
    e.Commit();
    ASSERT_WSTR_EQ(e.Retrieve(), L"o");
    return true;
}

bool test_bs_after_dd() {
    TelexEngine e;
    push(e, "dd");   // đ
    e.Backspace();   // → d
    e.Commit();
    ASSERT_WSTR_EQ(e.Retrieve(), L"d");
    return true;
}

bool test_bs_mid_word() {
    TelexEngine e;
    push(e, "viejt");  // việt
    e.Backspace();     // remove 't' → việ
    e.Commit();
    ASSERT_WSTR_EQ(e.Retrieve(), L"vi\x1ec7"); // việ
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
