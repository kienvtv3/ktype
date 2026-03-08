#include "test_helper.h"

bool test_viet()   { ASSERT_WSTR_EQ(commit("viejt"), L"vi\x1ec7t"); return true; }        // việt
bool test_nuoc()   { ASSERT_WSTR_EQ(commit("nuwowcs"), L"n\x01b0\x1edb" L"c"); return true; }  // nước
bool test_chao()   { ASSERT_WSTR_EQ(commit("chaof"), L"ch\x00e0o"); return true; }        // chào
bool test_dong()   { ASSERT_WSTR_EQ(commit("ddoong"), L"\x0111\x00f4ng"); return true; }  // đông
bool test_ddong2() { ASSERT_WSTR_EQ(commit("ddoongf"), L"\x0111\x1ed3ng"); return true; } // đồng
bool test_nguoi()  { ASSERT_WSTR_EQ(commit("nguwowif"), L"ng\x01b0\x1eddi"); return true; } // người
bool test_tieng()  { ASSERT_WSTR_EQ(commit("tiengs"), L"ti\x1ebfng"); return true; }      // tiếng
bool test_truong() { ASSERT_WSTR_EQ(commit("truwowngf"), L"tr\x01b0\x1eddng"); return true; } // trường
bool test_quoc()   { ASSERT_WSTR_EQ(commit("quoocs"), L"qu\x1ed1" L"c"); return true; }   // quốc
bool test_hoc()    { ASSERT_WSTR_EQ(commit("hocj"), L"h\x1ecd" L"c"); return true; }      // học
bool test_xin()    { ASSERT_WSTR_EQ(commit("xin"), L"xin"); return true; }
bool test_lam()    { ASSERT_WSTR_EQ(commit("lam"), L"lam"); return true; }
bool test_dep()    { ASSERT_WSTR_EQ(commit("dejp"), L"d\x1eb9p"); return true; }           // dẹp
bool test_ddep()   { ASSERT_WSTR_EQ(commit("ddejp"), L"\x0111\x1eb9p"); return true; }    // đẹp
bool test_anh()    { ASSERT_WSTR_EQ(commit("anh"), L"anh"); return true; }
bool test_em()     { ASSERT_WSTR_EQ(commit("em"), L"em"); return true; }
bool test_tot()    { ASSERT_WSTR_EQ(commit("toots"), L"t\x1ed1t"); return true; }          // tốt
bool test_nam()    { ASSERT_WSTR_EQ(commit("nawm"), L"n\x0103m"); return true; }           // năm
bool test_pho()    { ASSERT_WSTR_EQ(commit("phowf"), L"ph\x1edd"); return true; }         // phờ
bool test_gi()     { ASSERT_WSTR_EQ(commit("gias"), L"gi\x00e1"); return true; }           // giá

void run_word_tests() {
    printf("Word tests:\n");
    RUN_TEST(test_viet);
    RUN_TEST(test_nuoc);
    RUN_TEST(test_chao);
    RUN_TEST(test_dong);
    RUN_TEST(test_ddong2);
    RUN_TEST(test_nguoi);
    RUN_TEST(test_tieng);
    RUN_TEST(test_truong);
    RUN_TEST(test_quoc);
    RUN_TEST(test_hoc);
    RUN_TEST(test_xin);
    RUN_TEST(test_lam);
    RUN_TEST(test_dep);
    RUN_TEST(test_ddep);
    RUN_TEST(test_anh);
    RUN_TEST(test_em);
    RUN_TEST(test_tot);
    RUN_TEST(test_nam);
    RUN_TEST(test_pho);
    RUN_TEST(test_gi);
    printf("\n");
}
