#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "job_requirement.h"
#include "resume_processing.h"
#include "tokenizer.h"
#include "stopwords.h"
#include "analyzer.h"
#include "keyword_category.h"
#include "info_extractor.h"
#include "skill_match.h"
#include "scoring.h"
#include "ranking.h"
#include "shortlist.h"
#include "report.h"
#include "dashboard.h"
#include "test_helpers.h"

/*
 * Functional test: black-box, user-facing behavior. Rather than calling
 * individual module functions and checking their return values (that's
 * what the unit/integration suites do), this drives the exact same
 * sequence of public-API calls a real HR user triggers via the console
 * menu - upload requirement, upload resumes, process them, rank,
 * shortlist, generate reports - and checks the end result: does the right
 * candidate end up on top, and do the report files actually get written
 * with the right content?
 *
 * This mirrors ProcessResume() in main.c step for step (main.c's version
 * is `static` and can't be linked into the test binary directly, since
 * main.c owns its own main() which would collide with the test runner's).
 */

static void ProcessResumeForTest(const char *filepath)
{
    char *rawText = ReadResume(filepath);
    if (!rawText) return;

    Candidate *c = CURRENT;
    memset(c, 0, sizeof(Candidate));
    strncpy(c->filename, filepath, MAX_PATH_LEN - 1);

    ExtractName(rawText, c->name);
    ExtractEmail(rawText, c->email);
    ExtractPhone(rawText, c->phone);
    ExtractSkills(rawText);
    ExtractProjects(rawText);
    ExtractCertifications(rawText);
    ExtractEducation(rawText);
    ExtractExperience(rawText);

    char *workText = (char *)malloc(strlen(rawText) + 1);
    if (!workText) { free(rawText); return; }
    strcpy(workText, rawText);
    CleanText(workText);
    NormalizeWords(workText);

    static char tokens[MAX_TOKENS][MAX_TOKEN_LEN];
    int tokenCount = Tokenize(workText, tokens, MAX_TOKENS);
    c->totalWords = tokenCount;
    tokenCount = RemoveStopWords(tokens, tokenCount);

    CountWordFrequency(tokens, tokenCount);
    FindTopKeywords(10);
    ResumeStatistics();
    CategorizeKeywords();
    DomainDetection();

    MatchSkills();
    MissingSkills();
    CalculateMatchPercentage(c->matchedSkillCount, jobSkillCount);

    CalculateSkillScore();
    CalculateExperienceScore();
    CalculateEducationScore();
    CalculateProjectScore();
    CalculateCertificationScore();
    GenerateFinalScore();

    free(workText);
    free(rawText);

    candidateCount++;
    currentCandidateIdx++;
}

static int Init(void)
{
    ResetTestState();
    LoadFixtureTechSkills();
    LoadFixtureStopWords();
    LoadCategories("tests/fixtures/categories_test.txt");
    return 0;
}
static int Clean(void) { return 0; }

static void test_end_to_end_two_candidate_screening(void)
{
    ResetTestState();
    LoadFixtureTechSkills();
    LoadFixtureStopWords();
    LoadCategories("tests/fixtures/categories_test.txt");

    /* 1. HR loads the job requirement */
    int reqCount = UploadJobRequirement("tests/fixtures/job_requirement_test.txt");
    CU_ASSERT_EQUAL(reqCount, 5);

    /* 2. HR processes a strong and a weak candidate */
    ProcessResumeForTest("tests/fixtures/resume_strong.txt");
    ProcessResumeForTest("tests/fixtures/resume_weak.txt");
    CU_ASSERT_EQUAL(candidateCount, 2);

    /* 3. Candidates are ranked */
    RankCandidates();

    /* The strong candidate (5/5 required skills, degree, 2 projects, a
     * cert, 4 years experience) must clearly outrank the weak one (0/5
     * required skills, no matching tech stack). */
    CU_ASSERT_STRING_EQUAL(candidates[0].name, "ALICE STRONG");
    CU_ASSERT_TRUE(candidates[0].finalScore > candidates[1].finalScore);
    CU_ASSERT_EQUAL(candidates[0].rank, 1);
    CU_ASSERT_EQUAL(candidates[1].rank, 2);

    /* 4. HR sets 70% threshold: exact/above are automatically shortlisted. */
    int shortlisted = ShortlistByScoreThreshold(70);
    CU_ASSERT_EQUAL(shortlisted, 1);
    CU_ASSERT_TRUE(candidates[0].finalScore >= 70);
    CU_ASSERT_EQUAL(candidates[0].shortlisted, 1);
    CU_ASSERT_TRUE(candidates[1].finalScore < 70);
    CU_ASSERT_EQUAL(candidates[1].shortlisted, 0);
}

static void test_end_to_end_reports_are_written_with_correct_content(void)
{
    ResetTestState();
    LoadFixtureTechSkills();
    LoadFixtureStopWords();
    LoadCategories("tests/fixtures/categories_test.txt");

    UploadJobRequirement("tests/fixtures/job_requirement_test.txt");
    ProcessResumeForTest("tests/fixtures/resume_strong.txt");
    ProcessResumeForTest("tests/fixtures/resume_weak.txt");
    RankCandidates();
    ShortlistByScoreThreshold(70);
    GenerateShortlist();

    int csvRows = GenerateCSV("tests_tmp_report.csv");
    int txtRows = GenerateTXT("tests_tmp_report.txt");
    CU_ASSERT_EQUAL(csvRows, 2);
    CU_ASSERT_EQUAL(txtRows, 2);

    /* Confirm the CSV file actually contains the winning candidate's name */
    FILE *fp = fopen("tests_tmp_report.csv", "r");
    CU_ASSERT_PTR_NOT_NULL(fp);
    if (fp) {
        char buffer[4096] = "";
        size_t n = fread(buffer, 1, sizeof(buffer) - 1, fp);
        buffer[n] = '\0';
        fclose(fp);
        CU_ASSERT_TRUE(strstr(buffer, "ALICE STRONG") != NULL);
        CU_ASSERT_TRUE(strstr(buffer, "Automatic Shortlist Threshold, 70%") != NULL);
        CU_ASSERT_TRUE(strstr(buffer, "Shortlisted") != NULL);
    }

    FILE *sfp = fopen("output/shortlisted/ShortlistedCandidates.txt", "r");
    CU_ASSERT_PTR_NOT_NULL(sfp);
    if (sfp) fclose(sfp);

    remove("tests_tmp_report.csv");
    remove("tests_tmp_report.txt");
}

static void test_end_to_end_dashboard_reflects_real_data(void)
{
    ResetTestState();
    LoadFixtureTechSkills();
    LoadFixtureStopWords();
    LoadCategories("tests/fixtures/categories_test.txt");

    UploadJobRequirement("tests/fixtures/job_requirement_test.txt");
    ProcessResumeForTest("tests/fixtures/resume_strong.txt");
    ProcessResumeForTest("tests/fixtures/resume_weak.txt");
    RankCandidates();
    ShortlistByScoreThreshold(70);

    /* These just need to run without crashing against real processed data -
     * DisplayStatistics/DisplayTopSkills/DisplaySummary print to stdout and
     * don't return values to assert on directly. */
    DisplayStatistics();
    DisplayTopSkills();
    DisplaySummary();

    CU_ASSERT_EQUAL(candidateCount, 2);
}

void RegisterEndToEndFunctionalTests(void)
{
    CU_pSuite suite = CU_add_suite("Functional: end-to-end HR screening workflow", Init, Clean);
    CU_add_test(suite, "Two-candidate screening ranks the stronger candidate first", test_end_to_end_two_candidate_screening);
    CU_add_test(suite, "Generated reports contain the correct candidate data", test_end_to_end_reports_are_written_with_correct_content);
    CU_add_test(suite, "Dashboard views run correctly against real processed data", test_end_to_end_dashboard_reflects_real_data);
}
