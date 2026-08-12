#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <string.h>
#include "search.h"
#include "test_helpers.h"

static int Init(void)
{
    ResetTestState();

    Candidate *c1 = &candidates[0];
    strcpy(c1->name, "Vaishnavi M");
    strcpy(c1->email, "vaishnavi@example.com");
    strcpy(c1->degree, "B.E Computer Science");
    c1->experienceYears = 3;
    strcpy(c1->skills[0], "Python");
    strcpy(c1->skills[1], "React");
    c1->skillCount = 2;

    Candidate *c2 = &candidates[1];
    strcpy(c2->name, "Rahul Kumar");
    strcpy(c2->email, "rahul@example.com");
    strcpy(c2->degree, "BCA");
    c2->experienceYears = 1;
    strcpy(c2->skills[0], "Java");
    c2->skillCount = 1;

    candidateCount = 2;
    return 0;
}
static int Clean(void) { return 0; }

static void test_SearchByName_finds_partial_case_insensitive_match(void)
{
    CU_ASSERT_EQUAL(SearchByName("vaishnavi"), 1);
    CU_ASSERT_EQUAL(SearchByName("KUMAR"), 1);
}

static void test_SearchByName_no_match(void)
{
    CU_ASSERT_EQUAL(SearchByName("Nobody"), 0);
}

static void test_SearchBySkill_finds_matching_candidates(void)
{
    CU_ASSERT_EQUAL(SearchBySkill("Python"), 1);
    CU_ASSERT_EQUAL(SearchBySkill("java"), 1);
}

static void test_SearchBySkill_no_match(void)
{
    CU_ASSERT_EQUAL(SearchBySkill("Rust"), 0);
}

static void test_SearchByExperience_filters_by_minimum(void)
{
    CU_ASSERT_EQUAL(SearchByExperience(2), 1);  /* only Vaishnavi (3 yrs) */
    CU_ASSERT_EQUAL(SearchByExperience(0), 2);  /* both */
    CU_ASSERT_EQUAL(SearchByExperience(10), 0); /* neither */
}

static void test_SearchByEmail_exact_match(void)
{
    CU_ASSERT_EQUAL(SearchByEmail("vaishnavi@example.com"), 1);
    CU_ASSERT_EQUAL(SearchByEmail("nobody@example.com"), 0);
}

static void test_FilterByDegree_substring_match(void)
{
    CU_ASSERT_EQUAL(FilterByDegree("B.E"), 1);
    CU_ASSERT_EQUAL(FilterByDegree("BCA"), 1);
    CU_ASSERT_EQUAL(FilterByDegree("PhD"), 0);
}

void RegisterSearchTests(void)
{
    CU_pSuite suite = CU_add_suite("Unit: search", Init, Clean);
    CU_add_test(suite, "SearchByName finds a case-insensitive partial match", test_SearchByName_finds_partial_case_insensitive_match);
    CU_add_test(suite, "SearchByName returns 0 for no match", test_SearchByName_no_match);
    CU_add_test(suite, "SearchBySkill finds candidates with the skill", test_SearchBySkill_finds_matching_candidates);
    CU_add_test(suite, "SearchBySkill returns 0 for no match", test_SearchBySkill_no_match);
    CU_add_test(suite, "SearchByExperience filters by minimum years", test_SearchByExperience_filters_by_minimum);
    CU_add_test(suite, "SearchByEmail matches exact email", test_SearchByEmail_exact_match);
    CU_add_test(suite, "FilterByDegree matches on substring", test_FilterByDegree_substring_match);
}
