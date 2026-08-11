#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "tokenizer.h"
#include "analyzer.h"
#include "ranking.h"
#include "utils.h"
#include "test_helpers.h"
#include "parallel_reader.h"
#include "logger.h"

/*
 * Performance tests: these measure wall-clock time for the hottest code
 * paths against synthetic large inputs. Thresholds are deliberately
 * generous (measured in whole seconds) so the suite doesn't flake on a
 * slow CI runner - the goal is to catch a genuinely pathological
 * complexity bug (e.g. an accidental O(n^3)), not to chase milliseconds.
 * Actual timings are always printed so a human can spot a regression even
 * when the generous threshold still technically passes.
 */

static int Init(void) { ResetTestState(); return 0; }
static int Clean(void) { return 0; }

static double ElapsedSeconds(clock_t start, clock_t end)
{
    return (double)(end - start) / CLOCKS_PER_SEC;
}

/* Builds a large synthetic resume-like text: repeats a pool of realistic
 * words `wordCount` times total, so the size is controlled precisely. */
static void BuildLargeText(char *out, size_t outSize, int wordCount)
{
    static const char *pool[] = {
        "python", "developer", "react", "project", "experience",
        "mongodb", "database", "docker", "container", "git",
        "team", "built", "designed", "application", "backend"
    };
    size_t poolLen = sizeof(pool) / sizeof(pool[0]);
    out[0] = '\0';
    size_t used = 0;
    for (int i = 0; i < wordCount; i++) {
        const char *w = pool[i % poolLen];
        size_t wl = strlen(w);
        if (used + wl + 1 >= outSize) break;
        memcpy(out + used, w, wl);
        used += wl;
        out[used++] = ' ';
    }
    out[used] = '\0';
}

static void test_Tokenize_performance_on_large_text(void)
{
    static char bigText[100000];
    BuildLargeText(bigText, sizeof(bigText), 5000);

    static char tokens[MAX_TOKENS][MAX_TOKEN_LEN];

    clock_t start = clock();
    int count = Tokenize(bigText, tokens, MAX_TOKENS);
    clock_t end = clock();

    double seconds = ElapsedSeconds(start, end);
    printf("\n    [perf] Tokenize(%d words) took %.4fs", count, seconds);

    CU_ASSERT_TRUE(count > 0);
    CU_ASSERT_TRUE(seconds < 1.0);
}

static void test_CountWordFrequency_performance_on_large_token_set(void)
{
    static char bigText[100000];
    BuildLargeText(bigText, sizeof(bigText), 5000);
    static char tokens[MAX_TOKENS][MAX_TOKEN_LEN];
    int count = Tokenize(bigText, tokens, MAX_TOKENS);

    clock_t start = clock();
    int uniqueCount = CountWordFrequency(tokens, count);
    clock_t end = clock();

    double seconds = ElapsedSeconds(start, end);
    printf("\n    [perf] CountWordFrequency(%d tokens -> %d unique) took %.4fs", count, uniqueCount, seconds);

    CU_ASSERT_TRUE(uniqueCount > 0);
    CU_ASSERT_TRUE(seconds < 1.0);
}

static void test_RankCandidates_performance_at_max_capacity(void)
{
    /* Fill every candidate slot (MAX_CANDIDATES = 100) with a random-ish
     * score to exercise the sort at the project's real upper bound. */
    candidateCount = MAX_CANDIDATES;
    for (int i = 0; i < MAX_CANDIDATES; i++) {
        memset(&candidates[i], 0, sizeof(Candidate));
        snprintf(candidates[i].name, sizeof(candidates[i].name), "Candidate%d", i);
        candidates[i].finalScore = (i * 37) % 101; /* deterministic pseudo-random spread 0-100 */
    }

    clock_t start = clock();
    RankCandidates();
    clock_t end = clock();

    double seconds = ElapsedSeconds(start, end);
    printf("\n    [perf] RankCandidates(%d candidates) took %.4fs", MAX_CANDIDATES, seconds);

    /* Correctness alongside performance: verify the sort actually worked */
    int sortedCorrectly = 1;
    for (int i = 1; i < MAX_CANDIDATES; i++)
        if (candidates[i - 1].finalScore < candidates[i].finalScore) sortedCorrectly = 0;

    CU_ASSERT_TRUE(sortedCorrectly);
    CU_ASSERT_TRUE(seconds < 1.0);
}

static void test_ExtractKnownSkills_performance_with_full_skill_list(void)
{
    /* Worst case for the phrase-matcher: every one of MAX_TECH_SKILLS
     * entries has to be searched for against a large text. */
    techSkillCount = 0;
    for (int i = 0; i < MAX_TECH_SKILLS && i < MAX_TECH_SKILLS; i++) {
        snprintf(techSkills[techSkillCount], MAX_WORD_LEN, "skillword%d", i);
        techSkillCount++;
    }
    /* Make sure a handful actually appear in the text so the match path is exercised too */
    strcpy(techSkills[0], "python");
    strcpy(techSkills[1], "react");
    strcpy(techSkills[2], "docker");

    static char bigText[100000];
    BuildLargeText(bigText, sizeof(bigText), 5000);

    char out[MAX_TECH_SKILLS][MAX_WORD_LEN];
    int outCount = 0;

    clock_t start = clock();
    ExtractKnownSkills(bigText, out, &outCount, MAX_TECH_SKILLS);
    clock_t end = clock();

    double seconds = ElapsedSeconds(start, end);
    printf("\n    [perf] ExtractKnownSkills(%d skills x large text) took %.4fs, found %d", techSkillCount, seconds, outCount);

    CU_ASSERT_TRUE(outCount >= 3); /* python, react, docker should all be found */
    CU_ASSERT_TRUE(seconds < 2.0);
}


static void test_ParallelResumeProcessing_uses_worker_threads(void)
{
    char paths[4][MAX_PATH_LEN] = {
        "tests/fixtures/resume_strong.txt",
        "tests/fixtures/resume_weak.txt",
        "tests/fixtures/resume_strong.txt",
        "tests/fixtures/resume_weak.txt"
    };
    ResumeReadResult results[4];
    memset(results, 0, sizeof(results));

    clock_t start = clock();
    int threads = ReadResumesParallel(paths, 4, results, DEFAULT_RESUME_WORKER_THREADS);
    clock_t end = clock();

    double seconds = ElapsedSeconds(start, end);
    printf("\n    [perf] Parallel resume stage: %d resumes, %d worker threads, CPU time %.4fs",
           4, threads, seconds);

    CU_ASSERT_TRUE(threads >= 1);
    for (int i = 0; i < 4; i++) {
        CU_ASSERT_PTR_NOT_NULL(results[i].text);
        CU_ASSERT_PTR_NOT_NULL(results[i].tokens);
        CU_ASSERT_TRUE(results[i].tokenCount > 0);
        free(results[i].text);
        free(results[i].workText);
        free(results[i].tokens);
    }
}

void RegisterPerformanceTests(void)
{
    CU_pSuite suite = CU_add_suite("Performance: hot-path benchmarks", Init, Clean);
    CU_add_test(suite, "Tokenize handles a 5000-word text in reasonable time", test_Tokenize_performance_on_large_text);
    CU_add_test(suite, "CountWordFrequency handles a large token set in reasonable time", test_CountWordFrequency_performance_on_large_token_set);
    CU_add_test(suite, "RankCandidates sorts a full 100-candidate list in reasonable time", test_RankCandidates_performance_at_max_capacity);
    CU_add_test(suite, "ExtractKnownSkills matches against the full skill list in reasonable time", test_ExtractKnownSkills_performance_with_full_skill_list);
    CU_add_test(suite, "Parallel resume stage uses the worker pool safely", test_ParallelResumeProcessing_uses_worker_threads);
}
