#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <string.h>
#include "utils.h"
#include "test_helpers.h"

static int Init(void) { ResetTestState(); return 0; }
static int Clean(void) { return 0; }

static void test_TrimWhitespace_leading_and_trailing(void)
{
    char s[64] = "   hello world   ";
    TrimWhitespace(s);
    CU_ASSERT_STRING_EQUAL(s, "hello world");
}

static void test_TrimWhitespace_no_whitespace(void)
{
    char s[64] = "already-trimmed";
    TrimWhitespace(s);
    CU_ASSERT_STRING_EQUAL(s, "already-trimmed");
}

static void test_TrimWhitespace_all_whitespace(void)
{
    char s[64] = "     ";
    TrimWhitespace(s);
    CU_ASSERT_STRING_EQUAL(s, "");
}

static void test_ToLowerCase_mixed_case(void)
{
    char s[64] = "HeLLo WORLD 123";
    ToLowerCase(s);
    CU_ASSERT_STRING_EQUAL(s, "hello world 123");
}

static void test_FileExists_nonexistent_file(void)
{
    CU_ASSERT_FALSE(FileExists("this_file_should_not_exist_12345.txt"));
}

static void test_StrCaseCmp_case_insensitive_match(void)
{
    CU_ASSERT_EQUAL(StrCaseCmp("Python", "python"), 0);
    CU_ASSERT_EQUAL(StrCaseCmp("PYTHON", "PyThOn"), 0);
}

static void test_StrCaseCmp_different_strings(void)
{
    CU_ASSERT_NOT_EQUAL(StrCaseCmp("Python", "Java"), 0);
}

static void test_StrCaseContains_finds_substring(void)
{
    CU_ASSERT_TRUE(StrCaseContains("I know React and Node.js", "react"));
    CU_ASSERT_TRUE(StrCaseContains("B.E COMPUTER SCIENCE", "computer"));
}

static void test_StrCaseContains_no_match(void)
{
    CU_ASSERT_FALSE(StrCaseContains("I know React", "python"));
}

static void test_IsNumericToken_valid_numbers(void)
{
    CU_ASSERT_TRUE(IsNumericToken("9876543210"));
    CU_ASSERT_TRUE(IsNumericToken("0"));
}

static void test_IsNumericToken_rejects_non_numeric(void)
{
    CU_ASSERT_FALSE(IsNumericToken("98abc"));
    CU_ASSERT_FALSE(IsNumericToken(""));
    CU_ASSERT_FALSE(IsNumericToken(NULL));
}

/* --- ExtractKnownSkills: the phrase-aware matcher that fixed the
 *     "Java" false-positive inside "JavaScript" bug --- */

static void test_ExtractKnownSkills_matches_single_word_skills(void)
{
    LoadFixtureTechSkills();
    char out[MAX_JOB_SKILLS][MAX_WORD_LEN];
    int count = 0;
    ExtractKnownSkills("I have experience with Python and Docker.", out, &count, MAX_JOB_SKILLS);

    CU_ASSERT_EQUAL(count, 2);
}

static void test_ExtractKnownSkills_no_false_positive_java_in_javascript(void)
{
    LoadFixtureTechSkills();
    char out[MAX_JOB_SKILLS][MAX_WORD_LEN];
    int count = 0;
    ExtractKnownSkills("Skilled in JavaScript development.", out, &count, MAX_JOB_SKILLS);

    int foundJava = 0, foundJS = 0;
    for (int i = 0; i < count; i++) {
        if (StrCaseCmp(out[i], "Java") == 0) foundJava = 1;
        if (StrCaseCmp(out[i], "JavaScript") == 0) foundJS = 1;
    }
    CU_ASSERT_FALSE(foundJava);
    CU_ASSERT_TRUE(foundJS);
}

static void test_ExtractKnownSkills_matches_dotted_variant(void)
{
    LoadFixtureTechSkills();
    char out[MAX_JOB_SKILLS][MAX_WORD_LEN];
    int count = 0;
    ExtractKnownSkills("Built REST APIs using Node.js and MongoDB.", out, &count, MAX_JOB_SKILLS);

    int foundNode = 0, foundMongo = 0;
    for (int i = 0; i < count; i++) {
        if (StrCaseCmp(out[i], "Node.js") == 0) foundNode = 1;
        if (StrCaseCmp(out[i], "MongoDB") == 0) foundMongo = 1;
    }
    CU_ASSERT_TRUE(foundNode);
    CU_ASSERT_TRUE(foundMongo);
}

static void test_ExtractKnownSkills_no_duplicates(void)
{
    LoadFixtureTechSkills();
    char out[MAX_JOB_SKILLS][MAX_WORD_LEN];
    int count = 0;
    ExtractKnownSkills("Python Python PYTHON python", out, &count, MAX_JOB_SKILLS);
    CU_ASSERT_EQUAL(count, 1);
}

static void test_LoadWordListFile_reads_lines(void)
{
    FILE *fp = fopen("tests_tmp_wordlist.txt", "w");
    CU_ASSERT_PTR_NOT_NULL(fp);
    if (fp) {
        fprintf(fp, "alpha\nbeta\n\ngamma\n");
        fclose(fp);
    }

    char list[10][MAX_WORD_LEN];
    int count = 0;
    LoadWordListFile("tests_tmp_wordlist.txt", list, &count, 10);

    CU_ASSERT_EQUAL(count, 3);
    CU_ASSERT_STRING_EQUAL(list[0], "alpha");
    CU_ASSERT_STRING_EQUAL(list[1], "beta");
    CU_ASSERT_STRING_EQUAL(list[2], "gamma");

    remove("tests_tmp_wordlist.txt");
}

static void test_EnsureDirectoryExists_creates_missing_dir(void)
{
    remove("tests_tmp_dir");  /* harmless if it doesn't exist */
    EnsureDirectoryExists("tests_tmp_dir");
    CU_ASSERT_TRUE(FileExists("tests_tmp_dir"));
#ifdef _WIN32
    system("rmdir tests_tmp_dir");
#else
    system("rmdir tests_tmp_dir");
#endif
}

void RegisterUtilsTests(void)
{
    CU_pSuite suite = CU_add_suite("Unit: utils", Init, Clean);
    CU_add_test(suite, "TrimWhitespace removes leading/trailing spaces", test_TrimWhitespace_leading_and_trailing);
    CU_add_test(suite, "TrimWhitespace leaves clean string unchanged", test_TrimWhitespace_no_whitespace);
    CU_add_test(suite, "TrimWhitespace handles all-whitespace string", test_TrimWhitespace_all_whitespace);
    CU_add_test(suite, "ToLowerCase converts mixed-case text", test_ToLowerCase_mixed_case);
    CU_add_test(suite, "FileExists returns false for missing file", test_FileExists_nonexistent_file);
    CU_add_test(suite, "StrCaseCmp matches regardless of case", test_StrCaseCmp_case_insensitive_match);
    CU_add_test(suite, "StrCaseCmp distinguishes different strings", test_StrCaseCmp_different_strings);
    CU_add_test(suite, "StrCaseContains finds a case-insensitive substring", test_StrCaseContains_finds_substring);
    CU_add_test(suite, "StrCaseContains returns false when absent", test_StrCaseContains_no_match);
    CU_add_test(suite, "IsNumericToken accepts digit-only strings", test_IsNumericToken_valid_numbers);
    CU_add_test(suite, "IsNumericToken rejects non-numeric/empty/NULL", test_IsNumericToken_rejects_non_numeric);
    CU_add_test(suite, "ExtractKnownSkills matches known single-word skills", test_ExtractKnownSkills_matches_single_word_skills);
    CU_add_test(suite, "ExtractKnownSkills does not match Java inside JavaScript", test_ExtractKnownSkills_no_false_positive_java_in_javascript);
    CU_add_test(suite, "ExtractKnownSkills matches dotted variants like Node.js", test_ExtractKnownSkills_matches_dotted_variant);
    CU_add_test(suite, "ExtractKnownSkills de-duplicates repeated mentions", test_ExtractKnownSkills_no_duplicates);
    CU_add_test(suite, "LoadWordListFile reads non-blank lines", test_LoadWordListFile_reads_lines);
    CU_add_test(suite, "EnsureDirectoryExists creates a missing directory", test_EnsureDirectoryExists_creates_missing_dir);
}
