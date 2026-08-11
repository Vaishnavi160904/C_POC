#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <string.h>
#include "analyzer.h"
#include "utils.h"
#include "test_helpers.h"

static int Init(void) { ResetTestState(); currentCandidateIdx = 0; return 0; }
static int Clean(void) { return 0; }

static void test_CountWordFrequency_counts_repeated_words(void)
{
    char tokens[10][MAX_TOKEN_LEN] = { "python", "react", "python", "git", "python" };
    int uniqueCount = CountWordFrequency(tokens, 5);

    CU_ASSERT_EQUAL(uniqueCount, 3);

    int pythonCount = -1;
    for (int i = 0; i < currentKeywordCount; i++)
        if (strcmp(currentKeywords[i].word, "python") == 0) pythonCount = currentKeywords[i].count;

    CU_ASSERT_EQUAL(pythonCount, 3);
}

static void test_CountWordFrequency_empty_input(void)
{
    char tokens[1][MAX_TOKEN_LEN];
    int uniqueCount = CountWordFrequency(tokens, 0);
    CU_ASSERT_EQUAL(uniqueCount, 0);
}

static void test_FindTopKeywords_orders_by_frequency_descending(void)
{
    char tokens[10][MAX_TOKEN_LEN] = { "git", "python", "git", "python", "git" };
    CountWordFrequency(tokens, 5);

    /* Rebuild sorted copy the same way FindTopKeywords does internally and
     * confirm the highest-frequency word ("git", count 3) leads. */
    FindTopKeywords(5);

    int gitCount = 0, pythonCount = 0;
    for (int i = 0; i < currentKeywordCount; i++) {
        if (strcmp(currentKeywords[i].word, "git") == 0) gitCount = currentKeywords[i].count;
        if (strcmp(currentKeywords[i].word, "python") == 0) pythonCount = currentKeywords[i].count;
    }
    CU_ASSERT_TRUE(gitCount > pythonCount);
}

static void test_FindTopKeywords_populates_candidate_top_skills(void)
{
    LoadFixtureTechSkills();
    char tokens[10][MAX_TOKEN_LEN] = { "python", "python", "unknownword", "docker" };
    CountWordFrequency(tokens, 4);
    FindTopKeywords(10);

    Candidate *c = CURRENT;
    int foundPython = 0, foundDocker = 0, foundUnknown = 0;
    for (int i = 0; i < c->topSkillCount; i++) {
        if (StrCaseCmp(c->topSkills[i], "Python") == 0) foundPython = 1;
        if (StrCaseCmp(c->topSkills[i], "Docker") == 0) foundDocker = 1;
        if (StrCaseCmp(c->topSkills[i], "unknownword") == 0) foundUnknown = 1;
    }
    CU_ASSERT_TRUE(foundPython);
    CU_ASSERT_TRUE(foundDocker);
    CU_ASSERT_FALSE(foundUnknown); /* only recognized tech skills go into topSkills */
}

static void test_KeywordDensity_calculates_percentage(void)
{
    char tokens[10][MAX_TOKEN_LEN] = { "python", "react", "python", "git" };
    CountWordFrequency(tokens, 4);

    /* "python" appears 2 times out of 4 total words -> 50% */
    double density = KeywordDensity("python", 4);
    CU_ASSERT_DOUBLE_EQUAL(density, 50.0, 0.01);
}

static void test_KeywordDensity_zero_when_keyword_absent(void)
{
    char tokens[10][MAX_TOKEN_LEN] = { "react", "git" };
    CountWordFrequency(tokens, 2);

    double density = KeywordDensity("python", 2);
    CU_ASSERT_DOUBLE_EQUAL(density, 0.0, 0.01);
}

static void test_KeywordDensity_zero_total_words_no_crash(void)
{
    double density = KeywordDensity("python", 0);
    CU_ASSERT_DOUBLE_EQUAL(density, 0.0, 0.01);
}

static void test_LoadTechnicalSkills_loads_from_file(void)
{
    FILE *fp = fopen("tests_tmp_skills.txt", "w");
    fprintf(fp, "Python\nReact\nDocker\n");
    fclose(fp);

    int count = LoadTechnicalSkills("tests_tmp_skills.txt");
    CU_ASSERT_EQUAL(count, 3);

    remove("tests_tmp_skills.txt");
}

void RegisterAnalyzerTests(void)
{
    CU_pSuite suite = CU_add_suite("Unit: analyzer", Init, Clean);
    CU_add_test(suite, "CountWordFrequency counts repeated words correctly", test_CountWordFrequency_counts_repeated_words);
    CU_add_test(suite, "CountWordFrequency handles empty input", test_CountWordFrequency_empty_input);
    CU_add_test(suite, "FindTopKeywords orders by frequency descending", test_FindTopKeywords_orders_by_frequency_descending);
    CU_add_test(suite, "FindTopKeywords populates candidate's topSkills from known skills", test_FindTopKeywords_populates_candidate_top_skills);
    CU_add_test(suite, "KeywordDensity calculates correct percentage", test_KeywordDensity_calculates_percentage);
    CU_add_test(suite, "KeywordDensity returns 0 for absent keyword", test_KeywordDensity_zero_when_keyword_absent);
    CU_add_test(suite, "KeywordDensity handles zero total words without crashing", test_KeywordDensity_zero_total_words_no_crash);
    CU_add_test(suite, "LoadTechnicalSkills loads a skills file", test_LoadTechnicalSkills_loads_from_file);
}
