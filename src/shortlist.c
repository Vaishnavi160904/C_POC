#include "shortlist.h"
#include "utils.h"
#include "logger.h"

int SetShortlistThreshold(int threshold)
{
    if (threshold < 0 || threshold > 100) {
        LogWarning("SHORTLIST", "Rejected invalid shortlist threshold; valid range is 0-100");
        printf("[shortlist] Invalid threshold %d. Enter a value from 0 to 100.\n", threshold);
        return 0;
    }

    shortlistThreshold = threshold;
    char msg[128];
    snprintf(msg, sizeof(msg), "HR shortlist threshold set to %d%%", shortlistThreshold);
    LogInfo("SHORTLIST", msg);
    return 1;
}

int GetShortlistThreshold(void)
{
    return shortlistThreshold;
}

int ShortlistByScoreThreshold(int threshold)
{
    if (!SetShortlistThreshold(threshold))
        return 0;

    int selected = 0;
    for (int i = 0; i < candidateCount; i++) {
        candidates[i].shortlisted = (candidates[i].finalScore >= shortlistThreshold) ? 1 : 0;
        if (candidates[i].shortlisted)
            selected++;
    }

    char msg[192];
    snprintf(msg, sizeof(msg),
             "Automatic shortlisting completed: %d of %d candidate(s) scored >= %d%%",
             selected, candidateCount, shortlistThreshold);
    LogInfo("SHORTLIST", msg);

    printf("[shortlist] Automatic threshold: %d%%\n", shortlistThreshold);
    printf("[shortlist] %d of %d candidate(s) shortlisted (score >= %d%%)\n",
           selected, candidateCount, shortlistThreshold);
    return selected;
}

/* Assumes candidates[] is already sorted (RankCandidates() ran first). */
int SelectTopCandidates(int topN)
{
    if (topN <= 0) {
        printf("[shortlist] SelectTopCandidates: invalid count (%d), shortlisting 0 candidates\n", topN);
        topN = 0;
    }

    int selected = 0;
    for (int i = 0; i < candidateCount; i++) {
        if (i < topN) {
            candidates[i].shortlisted = 1;
            selected++;
        } else {
            candidates[i].shortlisted = 0;
        }
    }
    printf("[shortlist] SelectTopCandidates: %d of %d candidate(s) shortlisted\n", selected, candidateCount);
    return selected;
}

int GenerateShortlist(void)
{
    EnsureDirectoryExists("output");
    EnsureDirectoryExists("output/shortlisted");

    FILE *fp = fopen("output/shortlisted/ShortlistedCandidates.txt", "w");
    if (!fp) {
        printf("[shortlist] ERROR: could not open 'output/shortlisted/ShortlistedCandidates.txt' for writing\n");
        LogError("SHORTLIST", "Shortlist output file could not be opened");
    }

    int count = 0;
    printf("\n[shortlist] Automatically Shortlisted Candidates (threshold >= %d%%):\n",
           shortlistThreshold);

    if (fp) {
        fprintf(fp, "HR RESUME SCREENING - AUTOMATIC SHORTLIST\n");
        fprintf(fp, "Minimum Score Threshold: %d%%\n", shortlistThreshold);
        fprintf(fp, "Rule: candidates with Final Score >= threshold are shortlisted.\n\n");
    }

    for (int i = 0; i < candidateCount; i++) {
        if (candidates[i].shortlisted) {
            count++;
            printf("  %d. %s (Score: %d/100)\n", count, candidates[i].name, candidates[i].finalScore);
            if (fp) {
                fprintf(fp, "%d. %s - Score: %d/100 - %s\n",
                        count, candidates[i].name, candidates[i].finalScore, candidates[i].filename);
            }
        }
    }

    if (count == 0) {
        printf("  No candidate reached the %d%% threshold.\n", shortlistThreshold);
        if (fp) fprintf(fp, "No candidate reached the %d%% threshold.\n", shortlistThreshold);
    }

    if (fp) {
        fclose(fp);
        printf("[shortlist] Written to output/shortlisted/ShortlistedCandidates.txt\n");
        LogInfo("SHORTLIST", "Automatic shortlist file generated");
    }
    return count;
}
