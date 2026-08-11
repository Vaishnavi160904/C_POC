#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "scoring.h"
#include "test_helpers.h"

static int Init(void) { ResetTestState(); currentCandidateIdx = 0; return 0; }
static int Clean(void) { return 0; }

/* Weights per src/scoring.c: Skill 50, Experience 15 (3/yr, capped),
 * Education 10 (or 4 if no degree text), Projects 20 (7 each, capped),
 * Certifications 5 (2 each, capped) */

static void test_CalculateSkillScore_scales_with_match_percentage(void)
{
    CURRENT->matchPercentage = 100.0;
    CU_ASSERT_EQUAL(CalculateSkillScore(), 50);

    CURRENT->matchPercentage = 50.0;
    CU_ASSERT_EQUAL(CalculateSkillScore(), 25);

    CURRENT->matchPercentage = 0.0;
    CU_ASSERT_EQUAL(CalculateSkillScore(), 0);
}

static void test_CalculateExperienceScore_caps_at_15(void)
{
    CURRENT->experienceYears = 2;
    CU_ASSERT_EQUAL(CalculateExperienceScore(), 6);

    CURRENT->experienceYears = 10; /* would be 30 uncapped */
    CU_ASSERT_EQUAL(CalculateExperienceScore(), 15);

    CURRENT->experienceYears = 0;
    CU_ASSERT_EQUAL(CalculateExperienceScore(), 0);
}

static void test_CalculateEducationScore_with_and_without_degree(void)
{
    strcpy(CURRENT->degree, "B.Tech Computer Science");
    CU_ASSERT_EQUAL(CalculateEducationScore(), 10);

    CURRENT->degree[0] = '\0';
    CU_ASSERT_EQUAL(CalculateEducationScore(), 4);
}

static void test_CalculateProjectScore_caps_at_20(void)
{
    CURRENT->projectCount = 2;
    CU_ASSERT_EQUAL(CalculateProjectScore(), 14);

    CURRENT->projectCount = 5; /* would be 35 uncapped */
    CU_ASSERT_EQUAL(CalculateProjectScore(), 20);

    CURRENT->projectCount = 0;
    CU_ASSERT_EQUAL(CalculateProjectScore(), 0);
}

static void test_CalculateCertificationScore_caps_at_5(void)
{
    CURRENT->certCount = 1;
    CU_ASSERT_EQUAL(CalculateCertificationScore(), 2);

    CURRENT->certCount = 5; /* would be 10 uncapped */
    CU_ASSERT_EQUAL(CalculateCertificationScore(), 5);
}

static void test_GenerateFinalScore_sums_all_components(void)
{
    CURRENT->matchPercentage = 100.0;   /* -> 50 */
    CURRENT->experienceYears = 2;       /* -> 6  */
    strcpy(CURRENT->degree, "B.Tech");  /* -> 10 */
    CURRENT->projectCount = 2;          /* -> 14 */
    CURRENT->certCount = 1;             /* -> 2  */

    CalculateSkillScore();
    CalculateExperienceScore();
    CalculateEducationScore();
    CalculateProjectScore();
    CalculateCertificationScore();

    int finalScore = GenerateFinalScore();
    CU_ASSERT_EQUAL(finalScore, 50 + 6 + 10 + 14 + 2); /* 82 */
    CU_ASSERT_EQUAL(CURRENT->finalScore, 82);
}

static void test_GenerateFinalScore_never_exceeds_100(void)
{
    CURRENT->skillScore = 50;
    CURRENT->experienceScore = 15;
    CURRENT->educationScore = 10;
    CURRENT->projectScore = 20;
    CURRENT->certScore = 5; /* sums to exactly 100, sanity check the cap logic */

    int finalScore = GenerateFinalScore();
    CU_ASSERT_TRUE(finalScore <= 100);
}

void RegisterScoringTests(void)
{
    CU_pSuite suite = CU_add_suite("Unit: scoring", Init, Clean);
    CU_add_test(suite, "CalculateSkillScore scales linearly with match percentage", test_CalculateSkillScore_scales_with_match_percentage);
    CU_add_test(suite, "CalculateExperienceScore caps at 15", test_CalculateExperienceScore_caps_at_15);
    CU_add_test(suite, "CalculateEducationScore differs with/without a degree", test_CalculateEducationScore_with_and_without_degree);
    CU_add_test(suite, "CalculateProjectScore caps at 20", test_CalculateProjectScore_caps_at_20);
    CU_add_test(suite, "CalculateCertificationScore caps at 5", test_CalculateCertificationScore_caps_at_5);
    CU_add_test(suite, "GenerateFinalScore sums all five components correctly", test_GenerateFinalScore_sums_all_components);
    CU_add_test(suite, "GenerateFinalScore never exceeds 100", test_GenerateFinalScore_never_exceeds_100);
}
