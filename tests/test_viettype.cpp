#include "test_helper.h"

// Test cases adapted from VietType's TestTelex.cpp
// Source: VietType/VietTypeUnitTests/TestTelex.cpp

// Basic word typing (from VietType)
bool test_vt_ddas()     { ASSERT_WSTR_EQ(commit("ddas"), L"\x0111\x00e1"); return true; }            // đá
bool test_vt_nhuwonxg() { ASSERT_WSTR_EQ(commit("nhuwonxg"), L"nh\x01b0\x1ee1ng"); return true; }    // nhưỡng
bool test_vt_thoio()    { ASSERT_WSTR_EQ(commit("thoio"), L"th\x00f4i"); return true; }               // thôi (oio→ôi)
bool test_vt_nuaw()     { ASSERT_WSTR_EQ(commit("nuaw"), L"n\x01b0" L"a"); return true; }             // nưa
bool test_vt_quets()    { ASSERT_WSTR_EQ(commit("quets"), L"qu\x00e9t"); return true; }               // quét
bool test_vt_quitj()    { ASSERT_WSTR_EQ(commit("quitj"), L"qu\x1ecbt"); return true; }               // quịt
bool test_vt_queof()    { ASSERT_WSTR_EQ(commit("queof"), L"qu\x00e8o"); return true; }               // quèo
bool test_vt_thuees()   { ASSERT_WSTR_EQ(commit("thuees"), L"thu\x1ebf"); return true; }              // thuế
bool test_vt_ra()       { ASSERT_WSTR_EQ(commit("ra"), L"ra"); return true; }

// aua/aya transitions (alternative input order)
bool test_vt_lauar()    { ASSERT_WSTR_EQ(commit("lauar"), L"l\x1ea9u"); return true; }               // lẩu (aua→âu)
bool test_vt_nguayar()  { ASSERT_WSTR_EQ(commit("nguayar"), L"ngu\x1ea9y"); return true; }           // nguẩy (uaya→uây)
bool test_vt_luuw()     { ASSERT_WSTR_EQ(commit("luuw"), L"l\x01b0u"); return true; }                 // lưu (uu+w→ưu)

// gi handling (VietType)
bool test_vt_ginf()     { ASSERT_WSTR_EQ(commit("ginf"), L"g\x00ecn"); return true; }                 // gìn
bool test_vt_giowf()    { ASSERT_WSTR_EQ(commit("giowf"), L"gi\x1edd"); return true; }               // giờ
bool test_vt_giuwax()   { ASSERT_WSTR_EQ(commit("giuwax"), L"gi\x1eef" L"a"); return true; }           // giữa
bool test_vt_giuwx()    { ASSERT_WSTR_EQ(commit("giuwx"), L"gi\x1eef"); return true; }                // giữ (uw→ư, x=ngã)
bool test_vt_giemf()    { ASSERT_WSTR_EQ(commit("giemf"), L"gi\x00e8m"); return true; }               // giềm (e+huyền)

// Uppercase handling
bool test_vt_AAN()      { ASSERT_WSTR_EQ(commit("AAn"), L"\x00c2n"); return true; }                   // Ân
bool test_vt_Aan()      { ASSERT_WSTR_EQ(commit("Aan"), L"\x00c2n"); return true; }                   // Ân
bool test_vt_DDi()      { ASSERT_WSTR_EQ(commit("DDi"), L"\x0110i"); return true; }                   // Đi
bool test_vt_Ddi()      { ASSERT_WSTR_EQ(commit("Ddi"), L"\x0110i"); return true; }                   // Đi

// Invalid sequences (VietType double key tests)
bool test_vt_iis()      { ASSERT_WSTR_EQ(commit("iis"), L"iis"); return true; }                       // invalid
bool test_vt_system()   { ASSERT_WSTR_EQ(commit("system"), L"system"); return true; }                 // invalid

// Peek tests - intermediate state (VietType)
bool test_vt_peek_dd() {
    TelexEngine e;
    push(e, "dd");
    ASSERT_WSTR_EQ(e.Peek(), L"\x0111");  // đ
    return true;
}

bool test_vt_peek_ddd() {
    TelexEngine e;
    push(e, "ddd");
    ASSERT_WSTR_EQ(e.RetrieveRaw(), L"ddd");  // ddd (invalid, raw output)
    return true;
}

// Vowel after C2 (free doubling) — extended tests
bool test_vt_khongo() { ASSERT_WSTR_EQ(commit("khongo"), L"kh\x00f4ng"); return true; }              // không

// caasy → cấy (aa→â, s=sắc→ấ, y→ây)
bool test_vt_caasy() { ASSERT_WSTR_EQ(commit("caasy"), L"c\x1ea5y"); return true; }                   // cấy

void run_viettype_tests() {
    printf("VietType compatibility tests:\n");
    RUN_TEST(test_vt_ddas);
    RUN_TEST(test_vt_nhuwonxg);
    RUN_TEST(test_vt_thoio);
    RUN_TEST(test_vt_nuaw);
    RUN_TEST(test_vt_quets);
    RUN_TEST(test_vt_quitj);
    RUN_TEST(test_vt_queof);
    RUN_TEST(test_vt_thuees);
    RUN_TEST(test_vt_ra);
    RUN_TEST(test_vt_lauar);
    RUN_TEST(test_vt_nguayar);
    RUN_TEST(test_vt_luuw);
    RUN_TEST(test_vt_ginf);
    RUN_TEST(test_vt_giowf);
    RUN_TEST(test_vt_giuwax);
    RUN_TEST(test_vt_giuwx);
    RUN_TEST(test_vt_giemf);
    RUN_TEST(test_vt_AAN);
    RUN_TEST(test_vt_Aan);
    RUN_TEST(test_vt_DDi);
    RUN_TEST(test_vt_Ddi);
    RUN_TEST(test_vt_iis);
    RUN_TEST(test_vt_system);
    RUN_TEST(test_vt_peek_dd);
    RUN_TEST(test_vt_peek_ddd);
    RUN_TEST(test_vt_khongo);
    RUN_TEST(test_vt_caasy);
    printf("\n");
}
