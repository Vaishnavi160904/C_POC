#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "shortlist.h"
#include "test_helpers.h"

static int Init(void) { ResetTestState(); return 0; }
static int Clean(void) { return 0; }

static void test_threshold_includes_exact_score(void)
{
    candidateCount = 3;
    strcpy(candidates[0].name, "EXACT"); candidates[0].finalScore = 70;
    strcpy(candidates[1].name, "ABOVE"); candidates[1].finalScore = 85;
    strcpy(candidates[2].name, "BELOW"); candidates[2].finalScore = 69;

    int selected = ShortlistByScoreThreshold(70);
    CU_ASSERT_EQUAL(selected, 2);
    CU_ASSERT_TRUE(candidates[0].shortlisted);
    CU_ASSERT_TRUE(candidates[1].shortlisted);
    CU_ASSERT_FALSE(candidates[2].shortlisted);
}

static void test_invalid_threshold_is_rejected(void)
{
    CU_ASSERT_FALSE(SetShortlistThreshold(-1));
    CU_ASSERT_EQUAL(GetShortlistThreshold(), 70);
    CU_ASSERT_FALSE(SetShortlistThreshold(101));
    CU_ASSERT_EQUAL(GetShortlistThreshold(), 70);
}

static void test_zero_threshold_shortlists_every_candidate(void)
{
    candidateCount = 3;
    candidates[0].finalScore = 0;
    candidates[1].finalScore = 50;
    candidates[2].finalScore = 100;

    CU_ASSERT_EQUAL(ShortlistByScoreThreshold(0), 3);
}

void RegisterShortlistTests(void)
{
    CU_pSuite suite = CU_add_suite("Unit: score threshold shortlisting", Init, Clean);
    CU_add_test(suite, "70 percent includes exact and above scores", test_threshold_includes_exact_score);
    CU_add_test(suite, "invalid threshold is rejected", test_invalid_threshold_is_rejected);
    CU_add_test(suite, "zero threshold selects all candidates", test_zero_threshold_shortlists_every_candidate);
}
