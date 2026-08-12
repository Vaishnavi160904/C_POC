#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <string.h>
#include "stopwords.h"
#include "test_helpers.h"

static int Init(void) { ResetTestState(); return 0; }
static int Clean(void) { return 0; }

static void test_RemoveStopWords_filters_known_stopwords(void)
{
    LoadFixtureStopWords();
    char tokens[10][MAX_TOKEN_LEN] = { "the", "quick", "fox", "is", "fast" };
    int newCount = RemoveStopWords(tokens, 5);

    CU_ASSERT_EQUAL(newCount, 3);
    CU_ASSERT_STRING_EQUAL(tokens[0], "quick");
    CU_ASSERT_STRING_EQUAL(tokens[1], "fox");
    CU_ASSERT_STRING_EQUAL(tokens[2], "fast");
}

static void test_RemoveStopWords_is_case_insensitive(void)
{
    LoadFixtureStopWords();
    char tokens[10][MAX_TOKEN_LEN] = { "THE", "Python", "AND" };
    int newCount = RemoveStopWords(tokens, 3);

    CU_ASSERT_EQUAL(newCount, 1);
    CU_ASSERT_STRING_EQUAL(tokens[0], "Python");
}

static void test_RemoveStopWords_drops_single_character_tokens(void)
{
    LoadFixtureStopWords();
    char tokens[10][MAX_TOKEN_LEN] = { "a", "Python", "I" };
    int newCount = RemoveStopWords(tokens, 3);

    /* single-character tokens are dropped regardless of the stopword list */
    CU_ASSERT_EQUAL(newCount, 1);
    CU_ASSERT_STRING_EQUAL(tokens[0], "Python");
}

static void test_RemoveStopWords_keeps_all_when_no_stopwords_present(void)
{
    LoadFixtureStopWords();
    char tokens[10][MAX_TOKEN_LEN] = { "Python", "React", "MongoDB" };
    int newCount = RemoveStopWords(tokens, 3);
    CU_ASSERT_EQUAL(newCount, 3);
}

static void test_LoadStopWords_loads_from_file(void)
{
    FILE *fp = fopen("tests_tmp_stopwords.txt", "w");
    fprintf(fp, "the\nis\nand\n");
    fclose(fp);

    int count = LoadStopWords("tests_tmp_stopwords.txt");
    CU_ASSERT_EQUAL(count, 3);

    remove("tests_tmp_stopwords.txt");
}

void RegisterStopwordsTests(void)
{
    CU_pSuite suite = CU_add_suite("Unit: stopwords", Init, Clean);
    CU_add_test(suite, "RemoveStopWords filters known stop words", test_RemoveStopWords_filters_known_stopwords);
    CU_add_test(suite, "RemoveStopWords is case-insensitive", test_RemoveStopWords_is_case_insensitive);
    CU_add_test(suite, "RemoveStopWords drops single-character tokens", test_RemoveStopWords_drops_single_character_tokens);
    CU_add_test(suite, "RemoveStopWords keeps all tokens when none match", test_RemoveStopWords_keeps_all_when_no_stopwords_present);
    CU_add_test(suite, "LoadStopWords loads a stop word file", test_LoadStopWords_loads_from_file);
}
