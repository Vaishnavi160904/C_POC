/*
 * CUnit test runner for the HR Resume Screening System.
 *
 * Runs all four test categories in one pass:
 *   - Unit tests         (tests/unit/)         - one module in isolation
 *   - Integration tests   (tests/integration/)  - multiple modules together
 *   - Functional tests    (tests/functional/)   - full user-facing workflows
 *   - Performance tests    (tests/performance/)  - timing benchmarks
 *
 * IMPORTANT: run this from the project ROOT directory (the one containing
 * main.c, data/, output/), not from inside tests/ - several tests read
 * fixture files with paths like "tests/fixtures/..." and "data/skills.txt",
 * and auth tests back up/restore the real "data/users.dat".
 *
 * Exit code is the number of failed assertions (0 = all passed), which
 * makes this runner CI-friendly.
 */

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include "test_registry.h"

int main(void)
{
    if (CUE_SUCCESS != CU_initialize_registry()) {
        fprintf(stderr, "Failed to initialize CUnit registry\n");
        return (int)CU_get_error();
    }

    /* Unit */
    RegisterUtilsTests();
    RegisterCoverageBoostTests();
    RegisterAuthTests();
    RegisterTokenizerTests();
    RegisterStopwordsTests();
    RegisterAnalyzerTests();
    RegisterSkillMatchTests();
    RegisterScoringTests();
    RegisterRankingTests();
    RegisterSearchTests();
    RegisterShortlistTests();
    RegisterParallelReaderTests();

    /* Integration */
    RegisterPipelineIntegrationTests();
    RegisterRequirementMatchingIntegrationTests();

    /* Functional */
    RegisterEndToEndFunctionalTests();
    RegisterAccountJourneyFunctionalTests();

    /* Performance */
    RegisterPerformanceTests();

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    unsigned int failures = CU_get_number_of_failures();
    printf("\n=========================================================\n");
    if (failures == 0)
        printf(" ALL TESTS PASSED\n");
    else
        printf(" %u ASSERTION(S) FAILED\n", failures);
    printf("=========================================================\n");

    CU_cleanup_registry();
    return (int)failures;
}
