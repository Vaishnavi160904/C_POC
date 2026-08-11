#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <string.h>
#include "skill_match.h"
#include "utils.h"
#include "test_helpers.h"

static int Init(void)
{
    ResetTestState();
    currentCandidateIdx = 0;
    LoadFixtureJobSkills(); /* Python, React, MongoDB, Git, Docker */
    return 0;
}
static int Clean(void) { return 0; }

static void SetCandidateSkills(const char **skills, int count)
{
    Candidate *c = CURRENT;
    c->skillCount = 0;
    for (int i = 0; i < count; i++) {
        strncpy(c->skills[c->skillCount], skills[i], MAX_WORD_LEN - 1);
        c->skillCount++;
    }
}

static void test_MatchSkills_finds_all_overlapping_skills(void)
{
    const char *skills[] = { "Python", "React", "MongoDB", "Git", "Docker", "Extra" };
    SetCandidateSkills(skills, 6);

    int matched = MatchSkills();
    CU_ASSERT_EQUAL(matched, 5); /* all 5 job skills found, "Extra" is irrelevant */
}

static void test_MatchSkills_partial_overlap(void)
{
    const char *skills[] = { "Python", "Git" };
    SetCandidateSkills(skills, 2);

    int matched = MatchSkills();
    CU_ASSERT_EQUAL(matched, 2);
}

static void test_MatchSkills_case_insensitive(void)
{
    const char *skills[] = { "python", "REACT" };
    SetCandidateSkills(skills, 2);

    int matched = MatchSkills();
    CU_ASSERT_EQUAL(matched, 2);
}

static void test_MatchSkills_no_overlap(void)
{
    const char *skills[] = { "Flutter", "Firebase" };
    SetCandidateSkills(skills, 2);

    int matched = MatchSkills();
    CU_ASSERT_EQUAL(matched, 0);
}

static void test_MissingSkills_lists_unmatched_requirements(void)
{
    const char *skills[] = { "Python", "React" };
    SetCandidateSkills(skills, 2);

    MatchSkills();
    int missing = MissingSkills();

    CU_ASSERT_EQUAL(missing, 3); /* MongoDB, Git, Docker */

    Candidate *c = CURRENT;
    int foundMongo = 0;
    for (int i = 0; i < c->missingSkillCount; i++)
        if (StrCaseCmp(c->missingSkills[i], "MongoDB") == 0) foundMongo = 1;
    CU_ASSERT_TRUE(foundMongo);
}

static void test_CalculateMatchPercentage_correct_math(void)
{
    double pct = CalculateMatchPercentage(3, 5);
    CU_ASSERT_DOUBLE_EQUAL(pct, 60.0, 0.01);
    CU_ASSERT_DOUBLE_EQUAL(CURRENT->matchPercentage, 60.0, 0.01);
}

static void test_CalculateMatchPercentage_zero_total_no_crash(void)
{
    double pct = CalculateMatchPercentage(0, 0);
    CU_ASSERT_DOUBLE_EQUAL(pct, 0.0, 0.01);
}

void RegisterSkillMatchTests(void)
{
    CU_pSuite suite = CU_add_suite("Unit: skill_match", Init, Clean);
    CU_add_test(suite, "MatchSkills finds every overlapping skill", test_MatchSkills_finds_all_overlapping_skills);
    CU_add_test(suite, "MatchSkills handles partial overlap", test_MatchSkills_partial_overlap);
    CU_add_test(suite, "MatchSkills is case-insensitive", test_MatchSkills_case_insensitive);
    CU_add_test(suite, "MatchSkills returns 0 when there is no overlap", test_MatchSkills_no_overlap);
    CU_add_test(suite, "MissingSkills lists requirement skills not matched", test_MissingSkills_lists_unmatched_requirements);
    CU_add_test(suite, "CalculateMatchPercentage computes correct percentage", test_CalculateMatchPercentage_correct_math);
    CU_add_test(suite, "CalculateMatchPercentage handles zero total without crashing", test_CalculateMatchPercentage_zero_total_no_crash);
}
