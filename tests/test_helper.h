#pragma once

#include "telex.h"
#include <cstdio>
#include <string>

extern int g_total, g_passed, g_failed;

#define TEST_ASSERT(cond, msg) \
    do { if (!(cond)) { printf("  FAIL: %s (line %d)\n", msg, __LINE__); return false; } } while(0)

#define ASSERT_EQ(a, b) TEST_ASSERT((a) == (b), #a " == " #b)
#define ASSERT_TRUE(x) TEST_ASSERT((x), #x)
#define ASSERT_FALSE(x) TEST_ASSERT(!(x), "!" #x)

#define ASSERT_WSTR_EQ(a, b) \
    do { std::wstring _a = (a); std::wstring _b = (b); \
        if (_a != _b) { printf("  FAIL: strings differ (line %d)\n    expected: ", __LINE__); \
            for (auto _ch : _b) printf("%04x ", (int)_ch); printf("\n    got:      "); \
            for (auto _ch : _a) printf("%04x ", (int)_ch); printf("\n"); return false; } } while(0)

#define RUN_TEST(name) \
    do { g_total++; printf("  %s... ", #name); \
        if (name()) { g_passed++; printf("OK\n"); } else { g_failed++; } } while(0)

using namespace KType;

inline void push(TelexEngine& e, const char* s) {
    for (const char* p = s; *p; p++) {
        e.PushChar((wchar_t)*p);
    }
}

inline TelexStates push_state(TelexEngine& e, const char* s) {
    TelexStates state = TelexStates::Valid;
    for (const char* p = s; *p; p++) {
        state = e.PushChar((wchar_t)*p);
    }
    return state;
}

inline std::wstring commit(const char* input, const TelexConfig& cfg = {}) {
    TelexEngine e(cfg);
    push(e, input);
    e.Commit();
    return e.Retrieve();
}
