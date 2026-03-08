#include "test_helper.h"

bool test_viet()   { ASSERT_WSTR_EQ(commit("vieejt"), L"vi\x1ec7t"); return true; }       // việt (vieejt)
bool test_nuoc()   { ASSERT_WSTR_EQ(commit("nuwowcs"), L"n\x01b0\x1edb" L"c"); return true; }  // nước
bool test_chao()   { ASSERT_WSTR_EQ(commit("chaof"), L"ch\x00e0o"); return true; }        // chào
bool test_dong()   { ASSERT_WSTR_EQ(commit("ddoong"), L"\x0111\x00f4ng"); return true; }  // đông
bool test_ddong2() { ASSERT_WSTR_EQ(commit("ddoongf"), L"\x0111\x1ed3ng"); return true; } // đồng
bool test_nguoi()  { ASSERT_WSTR_EQ(commit("nguwowif"), L"ng\x01b0\x1eddi"); return true; } // người
bool test_tieng()  { ASSERT_WSTR_EQ(commit("tieengs"), L"ti\x1ebfng"); return true; }     // tiếng (tieengs)
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

// The three critical bug-fix tests
bool test_duoc()   { ASSERT_WSTR_EQ(commit("dduowcj"), L"\x0111\x01b0\x1ee3" L"c"); return true; }  // được
bool test_viet2()  { ASSERT_WSTR_EQ(commit("vieest"), L"vi\x1ebft"); return true; }                  // viết
bool test_nua()    { ASSERT_WSTR_EQ(commit("nuawx"), L"n\x1eef" L"a"); return true; }                // nữa
bool test_muon()   { ASSERT_WSTR_EQ(commit("muoons"), L"mu\x1ed1n"); return true; }                  // muốn

// Vowel transition after C2 (free doubling)
bool test_nen()    { ASSERT_WSTR_EQ(commit("nene"), L"n\x00ean"); return true; }                    // nên (nene)
bool test_toon()   { ASSERT_WSTR_EQ(commit("tono"), L"t\x00f4n"); return true; }                    // tôn (tono)

// Reverse circumflex: oo→ô, then ô+o→oo
bool test_xoong()  { ASSERT_WSTR_EQ(commit("xoong"), L"x\x00f4ng"); return true; }    // xoong → xông (oo→ô, ng)
bool test_xooong() { ASSERT_WSTR_EQ(commit("xooong"), L"xoong"); return true; }        // xooong → xoong (oo→ô, ô+o→oo, ng)
// WA after "qu" — quaw needs C2 to be valid
bool test_quaw()  { ASSERT_WSTR_EQ(commit("quaw"), L"quaw"); return true; }   // quă bare → invalid (MustC2)
bool test_quawt() { ASSERT_WSTR_EQ(commit("quawt"), L"qu\x0103t"); return true; }  // quăt → valid

// Vowel adjustments on C2 (transitions_wv_c2)
bool test_uwat()  { ASSERT_WSTR_EQ(commit("uwat"), L"u\x0103t"); return true; }                    // ưa+t → uăt (horn→breve)
bool test_uwon()  { ASSERT_WSTR_EQ(commit("uwon"), L"\x01b0\x01a1n"); return true; }               // ưo+n → ươn

// Tone same-key invalidates
bool test_tone_invalidate() { ASSERT_WSTR_EQ(commit("aff"), L"aff"); return true; }  // same tone twice → invalid

// Teencode exception: đ bypasses restricted C2 tone check
bool test_teencode() { ASSERT_WSTR_EQ(commit("ddejk"), L"\x0111\x1eb9k"); return true; }  // đẹk → valid (d-bar exemption)

// Leading W requires empty C1
bool test_nw_invalid() { ASSERT_WSTR_EQ(commit("nw"), L"nw"); return true; }  // nw → invalid (no LeadingW in default Telex)

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
    RUN_TEST(test_duoc);
    RUN_TEST(test_viet2);
    RUN_TEST(test_nua);
    RUN_TEST(test_muon);
    RUN_TEST(test_nen);
    RUN_TEST(test_toon);
    RUN_TEST(test_xoong);
    RUN_TEST(test_xooong);
    RUN_TEST(test_quaw);
    RUN_TEST(test_quawt);
    RUN_TEST(test_uwat);
    RUN_TEST(test_uwon);
    RUN_TEST(test_tone_invalidate);
    RUN_TEST(test_teencode);
    RUN_TEST(test_nw_invalid);
    printf("\n");
}
