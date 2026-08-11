#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parallel_reader.h"
#include "common.h"
#include "test_helpers.h"

static int Init(void) { ResetTestState(); return 0; }
static int Clean(void) { return 0; }

static void test_parallel_reader_processes_multiple_resumes(void)
{
    char paths[3][MAX_PATH_LEN] = {
        "tests/fixtures/resume_strong.txt",
        "tests/fixtures/resume_weak.txt",
        "tests/fixtures/resume_strong.txt"
    };
    ResumeReadResult results[3];
    memset(results, 0, sizeof(results));

    int threads = ReadResumesParallel(paths, 3, results, 3);

    CU_ASSERT_TRUE(threads >= 1);
    CU_ASSERT_PTR_NOT_NULL(results[0].text);
    CU_ASSERT_PTR_NOT_NULL(results[0].tokens);
    CU_ASSERT_TRUE(results[0].rawTokenCount > 0);
    CU_ASSERT_TRUE(results[0].tokenCount > 0);
    CU_ASSERT_PTR_NOT_NULL(results[1].text);
    CU_ASSERT_PTR_NOT_NULL(results[1].tokens);
    CU_ASSERT_TRUE(results[1].tokenCount > 0);

    for (int i = 0; i < 3; i++) {
        free(results[i].text);
        free(results[i].workText);
        free(results[i].tokens);
    }
}

static void test_parallel_reader_keeps_results_in_input_order(void)
{
    char paths[2][MAX_PATH_LEN] = {
        "tests/fixtures/resume_strong.txt",
        "tests/fixtures/resume_weak.txt"
    };
    ResumeReadResult results[2];
    memset(results, 0, sizeof(results));

    ReadResumesParallel(paths, 2, results, 2);

    CU_ASSERT_STRING_EQUAL(results[0].filepath, paths[0]);
    CU_ASSERT_STRING_EQUAL(results[1].filepath, paths[1]);

    for (int i = 0; i < 2; i++) {
        free(results[i].text);
        free(results[i].workText);
        free(results[i].tokens);
    }
}

void RegisterParallelReaderTests(void)
{
    CU_pSuite suite = CU_add_suite("Unit: parallel resume processing", Init, Clean);
    CU_add_test(suite, "Worker pool processes multiple resumes", test_parallel_reader_processes_multiple_resumes);
    CU_add_test(suite, "Parallel results preserve input order", test_parallel_reader_keeps_results_in_input_order);
}
