#include "dashboard.h"

void DisplayStatistics(void)
{
    int shortlisted = 0;
    long totalScore = 0;
    for (int i = 0; i < candidateCount; i++) {
        if (candidates[i].shortlisted) shortlisted++;
        totalScore += candidates[i].finalScore;
    }
    double avg = candidateCount > 0 ? (double)totalScore / candidateCount : 0.0;

    printf("\n[dashboard] ===== Resume Analytics Dashboard =====\n");
    printf("  Total Resumes Processed : %d\n", candidateCount);
    printf("  Shortlisted             : %d\n", shortlisted);
    printf("  Rejected                : %d\n", candidateCount - shortlisted);
    printf("  Average Candidate Score : %.1f\n", avg);
}

void DisplayTopSkills(void)
{
    KeywordFreq skillFreq[MAX_TECH_SKILLS];
    int freqCount = 0;

    for (int i = 0; i < candidateCount; i++) {
        for (int s = 0; s < candidates[i].skillCount; s++) {
            int found = -1;
            for (int f = 0; f < freqCount; f++)
                if (strcmp(skillFreq[f].word, candidates[i].skills[s]) == 0) { found = f; break; }
            if (found >= 0) {
                skillFreq[found].count++;
            } else if (freqCount < MAX_TECH_SKILLS) {
                strncpy(skillFreq[freqCount].word, candidates[i].skills[s], MAX_WORD_LEN - 1);
                skillFreq[freqCount].word[MAX_WORD_LEN - 1] = '\0';
                skillFreq[freqCount].count = 1;
                freqCount++;
            }
        }
    }

    for (int i = 0; i < freqCount - 1; i++) {
        int maxIdx = i;
        for (int j = i + 1; j < freqCount; j++)
            if (skillFreq[j].count > skillFreq[maxIdx].count) maxIdx = j;
        if (maxIdx != i) { KeywordFreq tmp = skillFreq[i]; skillFreq[i] = skillFreq[maxIdx]; skillFreq[maxIdx] = tmp; }
    }

    printf("\n[dashboard] Most Common Skills Across All Candidates:\n");
    int shown = freqCount < 10 ? freqCount : 10;
    for (int i = 0; i < shown; i++)
        printf("  %-20s appears in %d resume(s)\n", skillFreq[i].word, skillFreq[i].count);
    if (freqCount == 0) printf("  (no candidates processed yet)\n");
}

void DisplaySummary(void)
{
    printf("\n[dashboard] ===== Domain Distribution Summary =====\n");
    for (int i = 0; i < candidateCount; i++) {
        printf("  %-25s -> %s", candidates[i].name, candidates[i].primaryDomain);
        if (candidates[i].secondaryDomain[0]) printf(" / %s", candidates[i].secondaryDomain);
        printf("\n");
    }
    if (candidateCount == 0) printf("  (no candidates processed yet)\n");
}
