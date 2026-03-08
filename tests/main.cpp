#include "test_helper.h"

int g_total = 0, g_passed = 0, g_failed = 0;

void run_tone_tests();
void run_vowel_tests();
void run_consonant_tests();
void run_word_tests();
void run_edge_case_tests();
void run_backspace_tests();
void run_case_tests();

int main() {
    printf("=== KType Telex Engine Tests ===\n\n");

    run_tone_tests();
    run_vowel_tests();
    run_consonant_tests();
    run_word_tests();
    run_edge_case_tests();
    run_backspace_tests();
    run_case_tests();

    printf("\n=== Results: %d/%d passed", g_passed, g_total);
    if (g_failed > 0) printf(", %d FAILED", g_failed);
    printf(" ===\n");

    return g_failed > 0 ? 1 : 0;
}
