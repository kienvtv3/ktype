#include "test_helper.h"

// =============================================================================
// VietType compatibility tests — ported from VietType/VietTypeUnitTests/TestTelex.cpp
// Source: VietType commit (2026-03-09)
// Skipped sections: backconversion, autocorrect, abbreviations, TelexComplicated
// =============================================================================

// --- Word typing tests (SECTION "word typing tests") ---
bool test_vt_ddoongf()  { ASSERT_WSTR_EQ(commit("ddoongf"), L"\x0111\x1ed3ng"); return true; }   // đồng
bool test_vt_aans()     { ASSERT_WSTR_EQ(commit("aans"), L"\x1ea5n"); return true; }              // ấn
bool test_vt_ddas()     { ASSERT_WSTR_EQ(commit("ddas"), L"\x0111\x00e1"); return true; }         // đá
bool test_vt_nhuwonxg() { ASSERT_WSTR_EQ(commit("nhuwonxg"), L"nh\x01b0\x1ee1ng"); return true; } // nhưỡng
bool test_vt_nguiw()    { ASSERT_WSTR_EQ(commit("nguiw"), L"ng\x01b0i"); return true; }           // ngưi
bool test_vt_thoio()    { ASSERT_WSTR_EQ(commit("thoio"), L"th\x00f4i"); return true; }           // thôi
bool test_vt_nuaw()     { ASSERT_WSTR_EQ(commit("nuaw"), L"n\x01b0" L"a"); return true; }         // nưa
bool test_vt_quawms()   { ASSERT_WSTR_EQ(commit("quawms"), L"qu\x1eafm"); return true; }          // quắm
bool test_vt_quets()    { ASSERT_WSTR_EQ(commit("quets"), L"qu\x00e9t"); return true; }           // quét
bool test_vt_quauj()    { ASSERT_WSTR_EQ(commit("quauj"), L"qu\x1ea1u"); return true; }           // quạu
bool test_vt_quoj()     { ASSERT_WSTR_EQ(commit("quoj"), L"qu\x1ecd"); return true; }             // quọ
bool test_vt_quitj()    { ASSERT_WSTR_EQ(commit("quitj"), L"qu\x1ecbt"); return true; }           // quịt
bool test_vt_queof()    { ASSERT_WSTR_EQ(commit("queof"), L"qu\x00e8o"); return true; }           // quèo
bool test_vt_quowns()   { ASSERT_WSTR_EQ(commit("quowns"), L"qu\x1edbn"); return true; }          // quớn
bool test_vt_quwowns()  { ASSERT_WSTR_EQ(commit("quwowns"), L"q\x01b0\x1edbn"); return true; }    // qướn
bool test_vt_quonwx()   { ASSERT_WSTR_EQ(commit("quonwx"), L"qu\x1ee1n"); return true; }          // quỡn
bool test_vt_dduwowcj() { ASSERT_WSTR_EQ(commit("dduwowcj"), L"\x0111\x01b0\x1ee3" L"c"); return true; } // đượC
bool test_vt_nguwoif()  { ASSERT_WSTR_EQ(commit("nguwoif"), L"ng\x01b0\x1eddi"); return true; }   // người
bool test_vt_thuees()   { ASSERT_WSTR_EQ(commit("thuees"), L"thu\x1ebf"); return true; }          // thuế
bool test_vt_khuawngs() { ASSERT_WSTR_EQ(commit("khuawngs"), L"khu\x1eafng"); return true; }      // khuắng
bool test_vt_khuawsng() { ASSERT_WSTR_EQ(commit("khuawsng"), L"khu\x1eafng"); return true; }      // khuắng (tone before C2)
bool test_vt_ra()       { ASSERT_WSTR_EQ(commit("ra"), L"ra"); return true; }

// --- Variations of 'gi' (SECTION "test variations of 'gi'") ---
bool test_vt_gif()      { ASSERT_WSTR_EQ(commit("gif"), L"g\x00ec"); return true; }               // gì
bool test_vt_ginf()     { ASSERT_WSTR_EQ(commit("ginf"), L"g\x00ecn"); return true; }             // gìn
bool test_vt_giuowngf() { ASSERT_WSTR_EQ(commit("giuowngf"), L"gi\x01b0\x1eddng"); return true; } // giường
bool test_vt_giowf()    { ASSERT_WSTR_EQ(commit("giowf"), L"gi\x1edd"); return true; }            // giờ
bool test_vt_giuwax()   { ASSERT_WSTR_EQ(commit("giuwax"), L"gi\x1eef" L"a"); return true; }      // giữa
bool test_vt_giux()     { ASSERT_WSTR_EQ(commit("giux"), L"gi\x0169"); return true; }             // giũ
bool test_vt_giuoocj()  { ASSERT_WSTR_EQ(commit("giuoocj"), L"giu\x1ed9" L"c"); return true; }   // giuộc
bool test_vt_giemf()    { ASSERT_WSTR_EQ(commit("giemf"), L"gi\x00e8m"); return true; }           // giềm
bool test_vt_giee()     { ASSERT_WSTR_EQ(commit("giee"), L"gi\x00ea"); return true; }             // giê

// --- 'aua' and similar transitions (SECTION "test 'aua' and similar transitions") ---
bool test_vt_lauar()    { ASSERT_WSTR_EQ(commit("lauar"), L"l\x1ea9u"); return true; }            // lẩu
bool test_vt_nguayar()  { ASSERT_WSTR_EQ(commit("nguayar"), L"ngu\x1ea9y"); return true; }        // nguẩy
bool test_vt_luuw()     { ASSERT_WSTR_EQ(commit("luuw"), L"l\x01b0u"); return true; }             // lưu
bool test_vt_huouw()    { ASSERT_WSTR_EQ(commit("huouw"), L"h\x01b0\x01a1u"); return true; }      // hươu

// --- Irregular (SECTION "irregular") ---
bool test_vt_quoiws()   { ASSERT_WSTR_EQ(commit("quoiws"), L"qu\x1edbi"); return true; }          // quới
bool test_vt_ddawks()   { ASSERT_WSTR_EQ(commit("ddawks"), L"\x0111\x1eafk"); return true; }      // đắk

// --- Peek tests (SECTION "peek tests") ---
bool test_vt_peek_dd() {
    TelexEngine e;
    push(e, "dd");
    ASSERT_WSTR_EQ(e.Peek(), L"\x0111");  // đ
    return true;
}
bool test_vt_peek_ddd() {
    TelexEngine e;
    push(e, "ddd");
    ASSERT_WSTR_EQ(e.RetrieveRaw(), L"ddd");
    return true;
}
bool test_vt_peek_ad() {
    TelexEngine e;
    push(e, "ad");
    ASSERT_WSTR_EQ(e.Peek(), L"ad");
    return true;
}
bool test_vt_peek_quaw() {
    TelexEngine e;
    push(e, "quaw");
    ASSERT_WSTR_EQ(e.Peek(), L"qu\x0103");  // quă
    return true;
}
bool test_vt_peek_z() {
    TelexEngine e;
    push(e, "z");
    ASSERT_WSTR_EQ(e.Peek(), L"z");
    return true;
}
bool test_vt_peek_carc() {
    TelexEngine e;
    push(e, "carc");
    ASSERT_WSTR_EQ(e.Peek(), L"carc");
    return true;
}
bool test_vt_peek_ddark() {
    TelexEngine e;
    push(e, "ddark");
    ASSERT_WSTR_EQ(e.Peek(), L"\x0111\x1ea3k");  // đảk
    return true;
}
bool test_vt_peek_cace() {
    TelexEngine e;
    push(e, "cace");
    ASSERT_WSTR_EQ(e.Peek(), L"cace");
    return true;
}
bool test_vt_peek_nhaeng() {
    TelexEngine e;
    push(e, "nhaeng");
    ASSERT_WSTR_EQ(e.Peek(), L"nhaeng");
    return true;
}
bool test_vt_peek_cafe() {
    TelexEngine e;
    push(e, "cafe");
    ASSERT_WSTR_EQ(e.Peek(), L"cafe");
    return true;
}

// --- Double key tests (SECTION "double key tests") ---
bool test_vt_xuaaan()    { ASSERT_WSTR_EQ(commit("xuaaan"), L"xuaan"); return true; }
bool test_vt_iis()       { ASSERT_WSTR_EQ(commit("iis"), L"iis"); return true; }
bool test_vt_thooongf()  { ASSERT_WSTR_EQ(commit("thooongf"), L"tho\x00f2ng"); return true; }     // thòng
bool test_vt_thuongz()   { ASSERT_WSTR_EQ(commit("thuongz"), L"thuongz"); return true; }
bool test_vt_system()    { ASSERT_WSTR_EQ(commit("system"), L"system"); return true; }
bool test_vt_nhuwox()    { ASSERT_WSTR_EQ(commit("nhuwox"), L"nhuwox"); return true; }
// KNOWN GAP: VietType gives "tool", KType gives "toool" (needs _respos system)
// bool test_vt_toool()  { ASSERT_WSTR_EQ(commit("toool"), L"tool"); return true; }
bool test_vt_cuwowwcj()  { ASSERT_WSTR_EQ(commit("cuwowwcj"), L"cuwowcj"); return true; }        // W double → invalid
bool test_vt_quwowwns()  { ASSERT_WSTR_EQ(commit("quwowwns"), L"quwowns"); return true; }        // W double → invalid
bool test_vt_khongoo()   { ASSERT_WSTR_EQ(commit("khongoo"), L"khongo"); return true; }          // DoubleUndo: extra 'o' undoes oo→ô
bool test_vt_khongo()    { ASSERT_WSTR_EQ(commit("khongo"), L"kh\x00f4ng"); return true; }        // không
bool test_vt_caasy()     { ASSERT_WSTR_EQ(commit("caasy"), L"c\x1ea5y"); return true; }           // cấy

// --- Caps tests (SECTION "caps") ---
bool test_vt_peek_D() {
    TelexEngine e;
    e.PushChar(L'D');
    ASSERT_WSTR_EQ(e.Peek(), L"D");
    return true;
}
bool test_vt_XuOwngf() {
    ASSERT_WSTR_EQ(commit("XuOwngf"), L"X\x01b0\x1edcng");  // Xưởng (mixed case)
    return true;
}
bool test_vt_XuOnWfWx() {
    ASSERT_WSTR_EQ(commit("XuOnWfWx"), L"XuOnWfWx");  // invalid
    return true;
}
bool test_vt_NGUOIWF() {
    ASSERT_WSTR_EQ(commit("NGUOIWF"), L"NG\x01af\x1edcI");  // NGƯỜI
    return true;
}

// --- Caps repeat (SECTION "caps repeat") ---
// AAA and OWW already in test_vowels.cpp
bool test_vt_peek_OSS() {
    TelexEngine e;
    push(e, "OSS");
    ASSERT_WSTR_EQ(e.Peek(), L"OS");  // tone undo → raw "OS"
    return true;
}
bool test_vt_peek_GIFF() {
    TelexEngine e;
    push(e, "GIFF");
    ASSERT_WSTR_EQ(e.Peek(), L"GIF");  // tone undo → raw "GIF"
    return true;
}
bool test_vt_peek_GISS() {
    TelexEngine e;
    push(e, "GISS");
    ASSERT_WSTR_EQ(e.Peek(), L"GIS");  // tone undo → raw "GIS"
    return true;
}

// --- Uppercase word typing (SECTION "uppercase") ---
bool test_vt_AAN()  { ASSERT_WSTR_EQ(commit("AAn"), L"\x00c2n"); return true; }     // Ân
bool test_vt_Aan()  { ASSERT_WSTR_EQ(commit("Aan"), L"\x00c2n"); return true; }     // Ân
bool test_vt_DDi()  { ASSERT_WSTR_EQ(commit("DDi"), L"\x0110i"); return true; }     // Đi
bool test_vt_Ddi()  { ASSERT_WSTR_EQ(commit("Ddi"), L"\x0110i"); return true; }     // Đi
bool test_vt_AASn() { ASSERT_WSTR_EQ(commit("AASn"), L"\x1ea4n"); return true; }    // Ấn
bool test_vt_Aasn() { ASSERT_WSTR_EQ(commit("Aasn"), L"\x1ea4n"); return true; }    // Ấn
bool test_vt_AAsn() { ASSERT_WSTR_EQ(commit("AAsn"), L"\x1ea4n"); return true; }    // Ấn

// --- Backspace tests (SECTION "backspace tests") ---
bool test_vt_bs_ddoongf() {
    TelexEngine e;
    push(e, "ddoongf");
    ASSERT_EQ(e.Backspace(), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"\x0111\x1ed3n");  // đồn
    ASSERT_EQ(e.Backspace(), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"\x0111\x1ed3");   // đồ
    ASSERT_EQ(e.Backspace(), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"\x0111");          // đ
    ASSERT_EQ(e.Backspace(), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"");
    return true;
}

bool test_vt_bs_leeen() {
    TelexEngine e;
    push(e, "leeen");
    ASSERT_WSTR_EQ(e.Peek(), L"leen");  // eee→undo→"ee" (invalid), +n
    ASSERT_EQ(e.Backspace(), TelexStates::Invalid);
    ASSERT_WSTR_EQ(e.Peek(), L"lee");
    return true;
}

bool test_vt_bs_huowng() {
    TelexEngine e;
    push(e, "huowng");
    ASSERT_WSTR_EQ(e.Peek(), L"h\x01b0\x01a1ng");  // hương
    ASSERT_EQ(e.Backspace(), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"h\x01b0\x01a1n");   // hươn
    ASSERT_EQ(e.Backspace(), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"hu\x01a1");          // huơ
    ASSERT_EQ(e.Backspace(), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"hu");
    return true;
}

bool test_vt_bs_aoas() {
    TelexEngine e;
    push(e, "aoas");
    ASSERT_WSTR_EQ(e.Peek(), L"aoas");
    e.Backspace();
    ASSERT_WSTR_EQ(e.Peek(), L"aoa");
    return true;
}

bool test_vt_bs_heei() {
    TelexEngine e;
    push(e, "heei");
    ASSERT_WSTR_EQ(e.Peek(), L"heei");       // êi not valid vowel → raw
    e.Backspace();
    ASSERT_WSTR_EQ(e.Peek(), L"h\x00ea");   // hê
    return true;
}

bool test_vt_bs_owa() {
    TelexEngine e;
    push(e, "owa");
    ASSERT_WSTR_EQ(e.Peek(), L"owa");            // ơa not valid vowel → raw
    e.Backspace();
    ASSERT_WSTR_EQ(e.Peek(), L"\x01a1");        // ơ
    return true;
}

bool test_vt_bs_ruowi() {
    TelexEngine e;
    push(e, "ruowi");
    ASSERT_WSTR_EQ(e.Peek(), L"r\x01b0\x01a1i");  // rươi
    e.Backspace();
    ASSERT_WSTR_EQ(e.Peek(), L"ru\x01a1");         // ruơ
    return true;
}

bool test_vt_bs_quee() {
    TelexEngine e;
    push(e, "quee");
    ASSERT_WSTR_EQ(e.Peek(), L"qu\x00ea");  // quê
    e.Backspace();
    ASSERT_WSTR_EQ(e.Peek(), L"qu");
    return true;
}

bool test_vt_bs_quys() {
    TelexEngine e;
    push(e, "quys");
    ASSERT_WSTR_EQ(e.Peek(), L"qu\x00fd");  // quý
    e.Backspace();
    ASSERT_WSTR_EQ(e.Peek(), L"qu");
    return true;
}

bool test_vt_bs_hieef() {
    TelexEngine e;
    push(e, "hieef");
    ASSERT_WSTR_EQ(e.Peek(), L"hi\x1ec1");  // hiề
    e.Backspace();
    ASSERT_WSTR_EQ(e.Peek(), L"hi");
    return true;
}

bool test_vt_bs_gifg() {
    TelexEngine e;
    push(e, "gifg");
    ASSERT_WSTR_EQ(e.Peek(), L"gifg");       // raw (C2="g" invalid, preview matches commit)
    e.Backspace();
    ASSERT_WSTR_EQ(e.Peek(), L"g\x00ec");   // gì
    e.Backspace();
    ASSERT_WSTR_EQ(e.Peek(), L"g");
    e.PushChar(L'i');
    ASSERT_WSTR_EQ(e.Peek(), L"gi");
    return true;
}

bool test_vt_bs_ddayas() {
    TelexEngine e;
    push(e, "ddayas");
    ASSERT_WSTR_EQ(e.Peek(), L"\x0111\x1ea5y");  // đấy
    e.Backspace();
    ASSERT_WSTR_EQ(e.Peek(), L"\x0111\x1ea5");   // đấ
    e.Backspace();
    ASSERT_WSTR_EQ(e.Peek(), L"\x0111");          // đ
    return true;
}

bool test_vt_bs_xooong() {
    TelexEngine e;
    ASSERT_EQ(push_state(e, "xooong"), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"xoong");
    ASSERT_EQ(e.Backspace(), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"xoon");
    ASSERT_EQ(e.Backspace(), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"xoo");
    return true;
}

bool test_vt_bs_thooongf() {
    TelexEngine e;
    ASSERT_EQ(push_state(e, "thooongf"), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"tho\x00f2ng");  // thòng
    ASSERT_EQ(e.Backspace(), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"tho\x00f2n");   // thòn
    ASSERT_EQ(e.Backspace(), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"tho\x00f2");    // thò
    ASSERT_EQ(e.Backspace(), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"tho");
    return true;
}

// --- Tone and W movement backspace (SECTION "test tone and w movements") ---
bool test_vt_bs_cuwocs() {
    TelexEngine e;
    ASSERT_EQ(push_state(e, "cuwocs"), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"c\x01b0\x1edb" L"c");  // cước
    ASSERT_EQ(e.Backspace(), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"c\x01b0\x1edb");        // cướ
    ASSERT_EQ(e.Backspace(), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"c\x01b0");              // cư
    return true;
}

bool test_vt_bs_cuocws() {
    TelexEngine e;
    ASSERT_EQ(push_state(e, "cuocws"), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"c\x01b0\x1edb" L"c");  // cước
    ASSERT_EQ(e.Backspace(), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"cu\x1edb");             // cuớ (W removed, ơ still has tone)
    ASSERT_EQ(e.Backspace(), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"cu");
    return true;
}

bool test_vt_bs_cows() {
    TelexEngine e;
    ASSERT_EQ(push_state(e, "cows"), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"c\x1edb");  // cớ
    ASSERT_EQ(e.Backspace(), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"c");
    e.PushChar(L'u');
    ASSERT_WSTR_EQ(e.Peek(), L"cu");
    return true;
}

// --- Doublekey backspace (SECTION "test doublekey backspace") ---
bool test_vt_bs_mooo() {
    TelexEngine e;
    push(e, "mooo");
    ASSERT_EQ(e.Backspace(), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"mo");
    return true;
}

bool test_vt_bs_mooof() {
    TelexEngine e;
    push(e, "mooof");
    ASSERT_EQ(e.Backspace(), TelexStates::Valid);
    ASSERT_WSTR_EQ(e.Peek(), L"mo");
    return true;
}

// --- OA/OE/UY tone placement (SECTION "test oa/oe/uy") ---
bool test_vt_toanf()    { ASSERT_WSTR_EQ(commit("toanf"), L"to\x00e0n"); return true; }           // toàn
bool test_vt_hoaf_new() {
    TelexConfig c; c.oa_uy_tone1 = true;
    ASSERT_WSTR_EQ(commit("hoaf", c), L"ho\x00e0"); return true;  // hoà
}
bool test_vt_hoef_new() {
    TelexConfig c; c.oa_uy_tone1 = true;
    ASSERT_WSTR_EQ(commit("hoef", c), L"ho\x00e8"); return true;  // hoè
}
bool test_vt_luyj_new() {
    TelexConfig c; c.oa_uy_tone1 = true;
    ASSERT_WSTR_EQ(commit("luyj", c), L"lu\x1ef5"); return true;  // luỵ
}
bool test_vt_hoaf_old() {
    TelexConfig c; c.oa_uy_tone1 = false;
    ASSERT_WSTR_EQ(commit("hoaf", c), L"h\x00f2" L"a"); return true;  // hòa
}
bool test_vt_hoef_old() {
    TelexConfig c; c.oa_uy_tone1 = false;
    ASSERT_WSTR_EQ(commit("hoef", c), L"h\x00f2" L"e"); return true;  // hòe
}
bool test_vt_luyj_old() {
    TelexConfig c; c.oa_uy_tone1 = false;
    ASSERT_WSTR_EQ(commit("luyj", c), L"l\x1ee5y"); return true;  // lụy
}

// --- DD accept (SECTION "test dd accept") ---
bool test_vt_peek_dand() {
    TelexConfig c; c.accept_separate_dd = true;
    TelexEngine e(c);
    push(e, "dand");
    ASSERT_WSTR_EQ(e.Peek(), L"\x0111" L"an");  // đan
    return true;
}
bool test_vt_dodongf() {
    TelexConfig c; c.accept_separate_dd = true;
    ASSERT_WSTR_EQ(commit("dodongf", c), L"\x0111\x1ed3ng");  // đồng
    return true;
}

// --- Abbreviations (SECTION "test abbreviations" / "test no abbreviations") ---
bool test_vt_abbr_ddc() {
    ASSERT_WSTR_EQ(commit("ddc"), L"\x0111" L"c");  // đc
    return true;
}
bool test_vt_abbr_ddca_invalid() {
    ASSERT_WSTR_EQ(commit("ddca"), L"ddca");  // invalid — vowel after abbreviation
    return true;
}
bool test_vt_abbr_qdd() {
    ASSERT_WSTR_EQ(commit("qdd"), L"q\x0111");  // qđ
    return true;
}
bool test_vt_abbr_qdda_invalid() {
    ASSERT_WSTR_EQ(commit("qdda"), L"qdda");  // invalid — vowel after abbreviation
    return true;
}
bool test_vt_abbr_off_ddc() {
    TelexConfig c; c.allow_abbreviations = false;
    ASSERT_WSTR_EQ(commit("ddc", c), L"ddc");  // invalid when disabled
    return true;
}
bool test_vt_abbr_off_qdd() {
    TelexConfig c; c.allow_abbreviations = false;
    ASSERT_WSTR_EQ(commit("qdd", c), L"qdd");  // invalid when disabled
    return true;
}

// --- English word collision: optimize_multilang ---
bool test_vt_english_virus()  { TelexConfig c; c.optimize_multilang = 1; ASSERT_WSTR_EQ(commit("virus", c), L"virus"); return true; }
bool test_vt_english_horse()  { TelexConfig c; c.optimize_multilang = 1; ASSERT_WSTR_EQ(commit("horse", c), L"horse"); return true; }
bool test_vt_english_surf()   { TelexConfig c; c.optimize_multilang = 1; ASSERT_WSTR_EQ(commit("surf", c), L"surf"); return true; }
bool test_vt_english_doors()  { TelexConfig c; c.optimize_multilang = 1; ASSERT_WSTR_EQ(commit("doors", c), L"doors"); return true; }
bool test_vt_english_Horse()  { TelexConfig c; c.optimize_multilang = 1; ASSERT_WSTR_EQ(commit("Horse", c), L"Horse"); return true; }
bool test_vt_english_VIRUS()  { TelexConfig c; c.optimize_multilang = 1; ASSERT_WSTR_EQ(commit("VIRUS", c), L"VIRUS"); return true; }
bool test_vt_english_off()    { TelexConfig c; c.optimize_multilang = 0; ASSERT_WSTR_EQ(commit("virus", c), L"v\x00edu"); return true; }
bool test_vt_english_VIRUS_l1() { TelexConfig c; c.optimize_multilang = 1; ASSERT_WSTR_EQ(commit("VIRUS", c), L"VIRUS"); return true; }

// =============================================================================
void run_viettype_tests() {
    printf("VietType compatibility tests:\n");

    // Word typing
    RUN_TEST(test_vt_ddoongf);
    RUN_TEST(test_vt_aans);
    RUN_TEST(test_vt_ddas);
    RUN_TEST(test_vt_nhuwonxg);
    RUN_TEST(test_vt_nguiw);
    RUN_TEST(test_vt_thoio);
    RUN_TEST(test_vt_nuaw);
    RUN_TEST(test_vt_quawms);
    RUN_TEST(test_vt_quets);
    RUN_TEST(test_vt_quauj);
    RUN_TEST(test_vt_quoj);
    RUN_TEST(test_vt_quitj);
    RUN_TEST(test_vt_queof);
    RUN_TEST(test_vt_quowns);
    RUN_TEST(test_vt_quwowns);
    RUN_TEST(test_vt_quonwx);
    RUN_TEST(test_vt_dduwowcj);
    RUN_TEST(test_vt_nguwoif);
    RUN_TEST(test_vt_thuees);
    RUN_TEST(test_vt_khuawngs);
    RUN_TEST(test_vt_khuawsng);
    RUN_TEST(test_vt_ra);

    // gi variations
    RUN_TEST(test_vt_gif);
    RUN_TEST(test_vt_ginf);
    RUN_TEST(test_vt_giuowngf);
    RUN_TEST(test_vt_giowf);
    RUN_TEST(test_vt_giuwax);
    RUN_TEST(test_vt_giux);
    RUN_TEST(test_vt_giuoocj);
    RUN_TEST(test_vt_giemf);
    RUN_TEST(test_vt_giee);

    // aua transitions
    RUN_TEST(test_vt_lauar);
    RUN_TEST(test_vt_nguayar);
    RUN_TEST(test_vt_luuw);
    RUN_TEST(test_vt_huouw);

    // Irregular
    RUN_TEST(test_vt_quoiws);
    RUN_TEST(test_vt_ddawks);

    // Peek tests
    RUN_TEST(test_vt_peek_dd);
    RUN_TEST(test_vt_peek_ddd);
    RUN_TEST(test_vt_peek_ad);
    RUN_TEST(test_vt_peek_quaw);
    RUN_TEST(test_vt_peek_z);
    RUN_TEST(test_vt_peek_carc);
    RUN_TEST(test_vt_peek_ddark);
    RUN_TEST(test_vt_peek_cace);
    RUN_TEST(test_vt_peek_nhaeng);
    RUN_TEST(test_vt_peek_cafe);

    // Double key
    RUN_TEST(test_vt_xuaaan);
    RUN_TEST(test_vt_iis);
    RUN_TEST(test_vt_thooongf);
    RUN_TEST(test_vt_thuongz);
    RUN_TEST(test_vt_system);
    RUN_TEST(test_vt_nhuwox);
    RUN_TEST(test_vt_cuwowwcj);
    RUN_TEST(test_vt_quwowwns);
    RUN_TEST(test_vt_khongoo);
    RUN_TEST(test_vt_khongo);
    RUN_TEST(test_vt_caasy);

    // Caps
    RUN_TEST(test_vt_peek_D);
    RUN_TEST(test_vt_XuOwngf);
    RUN_TEST(test_vt_XuOnWfWx);
    RUN_TEST(test_vt_NGUOIWF);

    // Caps repeat
    RUN_TEST(test_vt_peek_OSS);
    RUN_TEST(test_vt_peek_GIFF);
    RUN_TEST(test_vt_peek_GISS);

    // Uppercase
    RUN_TEST(test_vt_AAN);
    RUN_TEST(test_vt_Aan);
    RUN_TEST(test_vt_DDi);
    RUN_TEST(test_vt_Ddi);
    RUN_TEST(test_vt_AASn);
    RUN_TEST(test_vt_Aasn);
    RUN_TEST(test_vt_AAsn);

    // Backspace
    RUN_TEST(test_vt_bs_ddoongf);
    RUN_TEST(test_vt_bs_leeen);
    RUN_TEST(test_vt_bs_huowng);
    RUN_TEST(test_vt_bs_aoas);
    RUN_TEST(test_vt_bs_heei);
    RUN_TEST(test_vt_bs_owa);
    RUN_TEST(test_vt_bs_ruowi);
    RUN_TEST(test_vt_bs_quee);
    RUN_TEST(test_vt_bs_quys);
    RUN_TEST(test_vt_bs_hieef);
    RUN_TEST(test_vt_bs_gifg);
    RUN_TEST(test_vt_bs_ddayas);
    RUN_TEST(test_vt_bs_xooong);
    RUN_TEST(test_vt_bs_thooongf);

    // Tone/W movement backspace
    RUN_TEST(test_vt_bs_cuwocs);
    RUN_TEST(test_vt_bs_cuocws);
    RUN_TEST(test_vt_bs_cows);

    // Doublekey backspace
    RUN_TEST(test_vt_bs_mooo);
    RUN_TEST(test_vt_bs_mooof);

    // OA/OE/UY
    RUN_TEST(test_vt_toanf);
    RUN_TEST(test_vt_hoaf_new);
    RUN_TEST(test_vt_hoef_new);
    RUN_TEST(test_vt_luyj_new);
    RUN_TEST(test_vt_hoaf_old);
    RUN_TEST(test_vt_hoef_old);
    RUN_TEST(test_vt_luyj_old);

    // DD accept
    RUN_TEST(test_vt_peek_dand);
    RUN_TEST(test_vt_dodongf);

    // Abbreviations
    RUN_TEST(test_vt_abbr_ddc);
    RUN_TEST(test_vt_abbr_ddca_invalid);
    RUN_TEST(test_vt_abbr_qdd);
    RUN_TEST(test_vt_abbr_qdda_invalid);
    RUN_TEST(test_vt_abbr_off_ddc);
    RUN_TEST(test_vt_abbr_off_qdd);

    // English multilang
    RUN_TEST(test_vt_english_virus);
    RUN_TEST(test_vt_english_horse);
    RUN_TEST(test_vt_english_surf);
    RUN_TEST(test_vt_english_doors);
    RUN_TEST(test_vt_english_Horse);
    RUN_TEST(test_vt_english_VIRUS);
    RUN_TEST(test_vt_english_off);
    RUN_TEST(test_vt_english_VIRUS_l1);

    printf("\n");
}
