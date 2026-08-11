#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <string.h>
#include "resume_processing.h"
#include "tokenizer.h"
#include "stopwords.h"
#include "analyzer.h"
#include "keyword_category.h"
#include "test_helpers.h"

/*
 * Integration test: exercises resume_processing + tokenizer + stopwords +
 * analyzer + keyword_category together, the way ProcessResume() in main.c
 * chains them - as opposed to the unit tests, which test each module in
 * isolation with hand-built fixture arrays.
 */

static int Init(void)
{
    ResetTestState();
    LoadFixtureTechSkills();
    LoadFixtureStopWords();
    return 0;
}
static int Clean(void) { return 0; }

static const char *SAMPLE_RESUME_TEXT =
    "VAISHNAVI M\n"
    "Skills\n"
    "Python React MongoDB Git Docker\n"
    "I have built a web application with Python and the React framework, "
    "using MongoDB for the database and Git for version control, all "
    "containerized with Docker.\n";

static void test_full_text_pipeline_produces_expected_keyword_counts(void)
{
    char workText[2000];
    strcpy(workText, SAMPLE_RESUME_TEXT);

    CleanText(workText);
    NormalizeWords(workText);

    char tokens[200][MAX_TOKEN_LEN];
    int tokenCount = Tokenize(workText, tokens, 200);
    CU_ASSERT_TRUE(tokenCount > 0);

    int afterStopwords = RemoveStopWords(tokens, tokenCount);
    CU_ASSERT_TRUE(afterStopwords <= tokenCount);
    CU_ASSERT_TRUE(afterStopwords > 0);

    int uniqueKeywords = CountWordFrequency(tokens, afterStopwords);
    CU_ASSERT_TRUE(uniqueKeywords > 0);

    /* "python" is mentioned twice in the sample text (once in the Skills
     * line, once in prose) - the pipeline should preserve that frequency
     * end to end through cleaning/tokenizing/stopword-removal. */
    int pythonCount = 0;
    for (int i = 0; i < currentKeywordCount; i++)
        if (strcmp(currentKeywords[i].word, "python") == 0) pythonCount = currentKeywords[i].count;
    CU_ASSERT_EQUAL(pythonCount, 2);
}

static void test_pipeline_feeds_categorization_correctly(void)
{
    LoadCategories("tests/fixtures/categories_test.txt");

    char workText[2000];
    strcpy(workText, SAMPLE_RESUME_TEXT);
    CleanText(workText);
    NormalizeWords(workText);

    char tokens[200][MAX_TOKEN_LEN];
    int tokenCount = Tokenize(workText, tokens, 200);
    tokenCount = RemoveStopWords(tokens, tokenCount);
    CountWordFrequency(tokens, tokenCount);

    CategorizeKeywords();

    /* The fixture categories file defines "Backend" containing Python and
     * "Frontend" containing React - both should register at least one match
     * since both words are in the sample resume text. */
    int backendMatches = -1, frontendMatches = -1;
    for (int i = 0; i < categoryCount; i++) {
        if (strcmp(categories[i].name, "Backend") == 0) backendMatches = categories[i].matchCount;
        if (strcmp(categories[i].name, "Frontend") == 0) frontendMatches = categories[i].matchCount;
    }
    CU_ASSERT_TRUE(backendMatches >= 1);
    CU_ASSERT_TRUE(frontendMatches >= 1);
}

static void test_pipeline_domain_detection_picks_highest_match_category(void)
{
    LoadCategories("tests/fixtures/categories_test.txt");
    currentCandidateIdx = 0;

    char workText[2000];
    strcpy(workText, SAMPLE_RESUME_TEXT);
    CleanText(workText);
    NormalizeWords(workText);

    char tokens[200][MAX_TOKEN_LEN];
    int tokenCount = Tokenize(workText, tokens, 200);
    tokenCount = RemoveStopWords(tokens, tokenCount);
    CountWordFrequency(tokens, tokenCount);
    CategorizeKeywords();
    DomainDetection();

    /* Some category should have been picked as the primary domain (not left
     * as the "General / Unclassified" fallback), since Backend/Frontend
     * both had real matches. */
    CU_ASSERT_TRUE(strcmp(CURRENT->primaryDomain, "General / Unclassified") != 0);
    CU_ASSERT_TRUE(strlen(CURRENT->primaryDomain) > 0);
}

static void test_ReadResume_txt_roundtrips_through_pipeline(void)
{
    FILE *fp = fopen("tests_tmp_resume.txt", "w");
    fprintf(fp, "%s", SAMPLE_RESUME_TEXT);
    fclose(fp);

    char *rawText = ReadResume("tests_tmp_resume.txt");
    CU_ASSERT_PTR_NOT_NULL(rawText);
    if (rawText) {
        CU_ASSERT_TRUE(strstr(rawText, "VAISHNAVI") != NULL);
        free(rawText);
    }

    remove("tests_tmp_resume.txt");
}

void RegisterPipelineIntegrationTests(void)
{
    CU_pSuite suite = CU_add_suite("Integration: text processing pipeline", Init, Clean);
    CU_add_test(suite, "Clean->Tokenize->Stopwords->Frequency preserves correct word counts", test_full_text_pipeline_produces_expected_keyword_counts);
    CU_add_test(suite, "Pipeline output feeds CategorizeKeywords correctly", test_pipeline_feeds_categorization_correctly);
    CU_add_test(suite, "Pipeline output feeds DomainDetection correctly", test_pipeline_domain_detection_picks_highest_match_category);
    CU_add_test(suite, "ReadResume(.txt) round-trips real file content into the pipeline", test_ReadResume_txt_roundtrips_through_pipeline);
}
