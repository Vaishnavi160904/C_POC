#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <string.h>
#include "ranking.h"
#include "test_helpers.h"

static int Init(void) { ResetTestState(); return 0; }
static int Clean(void) { return 0; }

static void AddCandidate(const char *name, int score)
{
    Candidate *c = &candidates[candidateCount];
    memset(c, 0, sizeof(Candidate));
    strncpy(c->name, name, sizeof(c->name) - 1);
    c->finalScore = score;
    candidateCount++;
}

static void test_RankCandidates_sorts_descending_by_score(void)
{
    ResetTestState();
    AddCandidate("Low", 40);
    AddCandidate("High", 90);
    AddCandidate("Mid", 65);

    RankCandidates();

    CU_ASSERT_STRING_EQUAL(candidates[0].name, "High");
    CU_ASSERT_STRING_EQUAL(candidates[1].name, "Mid");
    CU_ASSERT_STRING_EQUAL(candidates[2].name, "Low");
}

static void test_RankCandidates_assigns_sequential_rank_field(void)
{
    ResetTestState();
    AddCandidate("A", 30);
    AddCandidate("B", 80);

    RankCandidates();

    CU_ASSERT_EQUAL(candidates[0].rank, 1);
    CU_ASSERT_EQUAL(candidates[1].rank, 2);
}

static void test_RankCandidates_handles_tied_scores(void)
{
    ResetTestState();
    AddCandidate("First", 70);
    AddCandidate("Second", 70);

    RankCandidates();

    /* Stability isn't guaranteed by the sort, but both entries must still
     * be present with the correct score and consecutive ranks. */
    CU_ASSERT_EQUAL(candidates[0].finalScore, 70);
    CU_ASSERT_EQUAL(candidates[1].finalScore, 70);
    CU_ASSERT_EQUAL(candidates[0].rank, 1);
    CU_ASSERT_EQUAL(candidates[1].rank, 2);
}

static void test_RankCandidates_single_candidate(void)
{
    ResetTestState();
    AddCandidate("Solo", 55);
    RankCandidates();
    CU_ASSERT_EQUAL(candidates[0].rank, 1);
}

static void test_RankCandidates_empty_list_no_crash(void)
{
    ResetTestState();
    RankCandidates(); /* candidateCount is 0 - must not crash */
    CU_ASSERT_EQUAL(candidateCount, 0);
}

void RegisterRankingTests(void)
{
    CU_pSuite suite = CU_add_suite("Unit: ranking", Init, Clean);
    CU_add_test(suite, "RankCandidates sorts by final score descending", test_RankCandidates_sorts_descending_by_score);
    CU_add_test(suite, "RankCandidates assigns sequential rank numbers", test_RankCandidates_assigns_sequential_rank_field);
    CU_add_test(suite, "RankCandidates handles tied scores without losing data", test_RankCandidates_handles_tied_scores);
    CU_add_test(suite, "RankCandidates works correctly with a single candidate", test_RankCandidates_single_candidate);
    CU_add_test(suite, "RankCandidates does not crash on an empty list", test_RankCandidates_empty_list_no_crash);
}
