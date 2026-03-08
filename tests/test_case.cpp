#include "test_helper.h"

// Uppercase first letter
bool test_upper_first() {
    TelexEngine e;
    e.PushChar(L'V');
    push(e, "ieejt");
    e.Commit();
    ASSERT_WSTR_EQ(e.Retrieve(), L"Vi\x1ec7t");
    return true;
}

// All uppercase
bool test_all_upper() {
    TelexEngine e;
    e.PushChar(L'V');
    e.PushChar(L'I');
    e.PushChar(L'E');
    e.PushChar(L'E');
    e.PushChar(L'J');
    e.PushChar(L'T');
    e.Commit();
    std::wstring result = e.Retrieve();
    ASSERT_WSTR_EQ(result, L"VI\x1ec6T");
    return true;
}

// Uppercase dd
bool test_upper_dd() {
    TelexEngine e;
    e.PushChar(L'D');
    e.PushChar(L'D');
    push(e, "a");
    e.Commit();
    std::wstring result = e.Retrieve();
    ASSERT_WSTR_EQ(result, L"\x0110" L"a");
    return true;
}

// All lowercase
bool test_lower() {
    ASSERT_WSTR_EQ(commit("vieejt"), L"vi\x1ec7t");
    return true;
}

void run_case_tests() {
    printf("Case tests:\n");
    RUN_TEST(test_upper_first);
    RUN_TEST(test_all_upper);
    RUN_TEST(test_upper_dd);
    RUN_TEST(test_lower);
    printf("\n");
}
