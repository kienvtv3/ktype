#include "test_helper.h"

// dd → đ
bool test_dd() { ASSERT_WSTR_EQ(commit("dda"), L"\x0111" L"a"); return true; }  // đa
bool test_dd_only() { ASSERT_WSTR_EQ(commit("dd"), L"\x0111"); return true; }   // đ (valid C1)

// C1 consonant clusters
bool test_c1_b()   { ASSERT_WSTR_EQ(commit("ba"),  L"ba");  return true; }
bool test_c1_ch()  { ASSERT_WSTR_EQ(commit("cha"), L"cha"); return true; }
bool test_c1_gh()  { ASSERT_WSTR_EQ(commit("gha"), L"gha"); return true; }
bool test_c1_gi()  { ASSERT_WSTR_EQ(commit("gia"), L"gia"); return true; }
bool test_c1_kh()  { ASSERT_WSTR_EQ(commit("kha"), L"kha"); return true; }
bool test_c1_ng()  { ASSERT_WSTR_EQ(commit("nga"), L"nga"); return true; }
bool test_c1_ngh() { ASSERT_WSTR_EQ(commit("nghe"), L"nghe"); return true; }
bool test_c1_nh()  { ASSERT_WSTR_EQ(commit("nha"), L"nha"); return true; }
bool test_c1_ph()  { ASSERT_WSTR_EQ(commit("pha"), L"pha"); return true; }
bool test_c1_qu()  { ASSERT_WSTR_EQ(commit("qua"), L"qua"); return true; }
bool test_c1_th()  { ASSERT_WSTR_EQ(commit("tha"), L"tha"); return true; }
bool test_c1_tr()  { ASSERT_WSTR_EQ(commit("tra"), L"tra"); return true; }

// C2 consonant codas
bool test_c2_c()  { ASSERT_WSTR_EQ(commit("ac"),  L"ac");  return true; }
bool test_c2_ch() { ASSERT_WSTR_EQ(commit("ach"), L"ach"); return true; }
bool test_c2_k()  { ASSERT_WSTR_EQ(commit("ak"),  L"ak");  return true; }
bool test_c2_m()  { ASSERT_WSTR_EQ(commit("am"),  L"am");  return true; }
bool test_c2_n()  { ASSERT_WSTR_EQ(commit("an"),  L"an");  return true; }
bool test_c2_ng() { ASSERT_WSTR_EQ(commit("ang"), L"ang"); return true; }
bool test_c2_nh() { ASSERT_WSTR_EQ(commit("anh"), L"anh"); return true; }
bool test_c2_p()  { ASSERT_WSTR_EQ(commit("ap"),  L"ap");  return true; }
bool test_c2_t()  { ASSERT_WSTR_EQ(commit("at"),  L"at");  return true; }

// Restricted C2: only sắc(S) or nặng(J) allowed
bool test_restricted_c2_sac()   { ASSERT_WSTR_EQ(commit("acs"), L"\x00e1" L"c"); return true; }  // ác
bool test_restricted_c2_nang()  { ASSERT_WSTR_EQ(commit("acj"), L"\x1ea1" L"c"); return true; }  // ạc
bool test_restricted_c2_huyen() { ASSERT_WSTR_EQ(commit("acf"), L"acf"); return true; }          // invalid → raw

// Invalid C1 clusters
bool test_invalid_c1() {
    TelexEngine e;
    e.PushChar(L'b');
    e.PushChar(L'x');
    ASSERT_EQ(e.PushChar(L'a'), TelexStates::Invalid);
    return true;
}

void run_consonant_tests() {
    printf("Consonant tests:\n");
    RUN_TEST(test_dd);
    RUN_TEST(test_dd_only);
    RUN_TEST(test_c1_b);
    RUN_TEST(test_c1_ch);
    RUN_TEST(test_c1_gh);
    RUN_TEST(test_c1_gi);
    RUN_TEST(test_c1_kh);
    RUN_TEST(test_c1_ng);
    RUN_TEST(test_c1_ngh);
    RUN_TEST(test_c1_nh);
    RUN_TEST(test_c1_ph);
    RUN_TEST(test_c1_qu);
    RUN_TEST(test_c1_th);
    RUN_TEST(test_c1_tr);
    RUN_TEST(test_c2_c);
    RUN_TEST(test_c2_ch);
    RUN_TEST(test_c2_k);
    RUN_TEST(test_c2_m);
    RUN_TEST(test_c2_n);
    RUN_TEST(test_c2_ng);
    RUN_TEST(test_c2_nh);
    RUN_TEST(test_c2_p);
    RUN_TEST(test_c2_t);
    RUN_TEST(test_restricted_c2_sac);
    RUN_TEST(test_restricted_c2_nang);
    RUN_TEST(test_restricted_c2_huyen);
    RUN_TEST(test_invalid_c1);
    printf("\n");
}
