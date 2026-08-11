#include "ranking.h"

/* Insertion sort by finalScore, descending. candidateCount is small enough
 * (resume counts in the tens/hundreds) that O(n^2) is perfectly fine here. */
/* Only used internally by RankCandidates() - not part of the public module API. */
static void SortCandidates(void)
{
    for (int i = 1; i < candidateCount; i++) {
        Candidate key = candidates[i];
        int j = i - 1;
        while (j >= 0 && candidates[j].finalScore < key.finalScore) {
            candidates[j + 1] = candidates[j];
            j--;
        }
        candidates[j + 1] = key;
    }
    printf("[ranking] SortCandidates: %d candidate(s) sorted by final score\n", candidateCount);
}

void RankCandidates(void)
{
    SortCandidates();
    printf("\n[ranking] Candidate Ranking:\n");
    for (int i = 0; i < candidateCount; i++) {
        candidates[i].rank = i + 1;
        printf("  #%-2d %-25s Score: %3d/100\n", candidates[i].rank, candidates[i].name, candidates[i].finalScore);
    }
}
