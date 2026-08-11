#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <string.h>
#include "tokenizer.h"
#include "test_helpers.h"

static int Init(void) { ResetTestState(); return 0; }
static int Clean(void) { return 0; }

static void test_Tokenize_splits_on_whitespace(void)
{
    char tokens[50][MAX_TOKEN_LEN];
    int count = Tokenize("Python React MongoDB", tokens, 50);

    CU_ASSERT_EQUAL(count, 3);
    CU_ASSERT_STRING_EQUAL(tokens[0], "Python");
    CU_ASSERT_STRING_EQUAL(tokens[1], "React");
    CU_ASSERT_STRING_EQUAL(tokens[2], "MongoDB");
}

static void test_Tokenize_handles_multiple_spaces_tabs_newlines(void)
{
    char tokens[50][MAX_TOKEN_LEN];
    int count = Tokenize("Python\t\tReact\n\nMongoDB   Git", tokens, 50);
    CU_ASSERT_EQUAL(count, 4);
}

static void test_Tokenize_empty_string(void)
{
    char tokens[50][MAX_TOKEN_LEN];
    int count = Tokenize("", tokens, 50);
    CU_ASSERT_EQUAL(count, 0);
}

static void test_Tokenize_null_text_returns_zero(void)
{
    char tokens[50][MAX_TOKEN_LEN];
    int count = Tokenize(NULL, tokens, 50);
    CU_ASSERT_EQUAL(count, 0);
}

static void test_Tokenize_respects_maxTokens_limit(void)
{
    char tokens[3][MAX_TOKEN_LEN];
    int count = Tokenize("one two three four five", tokens, 3);
    CU_ASSERT_EQUAL(count, 3);
}

static void test_Tokenize_does_not_modify_caller_buffer(void)
{
    /* Tokenize must work off a private copy - the original text pointer
     * passed in should be untouched (it's declared const for exactly this
     * reason), unlike raw strtok() which mutates its input in place. */
    const char *original = "Python React MongoDB";
    char textCopy[64];
    strcpy(textCopy, original);

    char tokens[50][MAX_TOKEN_LEN];
    Tokenize(textCopy, tokens, 50);

    CU_ASSERT_STRING_EQUAL(textCopy, original);
}

void RegisterTokenizerTests(void)
{
    CU_pSuite suite = CU_add_suite("Unit: tokenizer", Init, Clean);
    CU_add_test(suite, "Tokenize splits on whitespace", test_Tokenize_splits_on_whitespace);
    CU_add_test(suite, "Tokenize handles tabs/newlines/multiple spaces", test_Tokenize_handles_multiple_spaces_tabs_newlines);
    CU_add_test(suite, "Tokenize returns 0 for empty string", test_Tokenize_empty_string);
    CU_add_test(suite, "Tokenize returns 0 for NULL text", test_Tokenize_null_text_returns_zero);
    CU_add_test(suite, "Tokenize respects maxTokens limit", test_Tokenize_respects_maxTokens_limit);
    CU_add_test(suite, "Tokenize does not mutate the caller's buffer", test_Tokenize_does_not_modify_caller_buffer);
}
