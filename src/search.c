#include "search.h"
#include "utils.h"

int SearchByName(const char *name)
{
    int matches = 0;
    printf("\n[search] SearchByName('%s'):\n", name);
    for (int i = 0; i < candidateCount; i++) {
        if (StrCaseContains(candidates[i].name, name)) {
            printf("  Found: %s | Score: %d | Rank: %d\n",
                   candidates[i].name, candidates[i].finalScore, candidates[i].rank);
            matches++;
        }
    }
    if (!matches) printf("  No matches\n");
    return matches;
}

int SearchBySkill(const char *skill)
{
    int matches = 0;
    printf("\n[search] SearchBySkill('%s'):\n", skill);
    for (int i = 0; i < candidateCount; i++) {
        for (int s = 0; s < candidates[i].skillCount; s++) {
            if (StrCaseCmp(candidates[i].skills[s], skill) == 0) {
                printf("  Found: %s\n", candidates[i].name);
                matches++;
                break;
            }
        }
    }
    if (!matches) printf("  No matches\n");
    return matches;
}

int SearchByEmail(const char *email)
{
    for (int i = 0; i < candidateCount; i++) {
        if (StrCaseCmp(candidates[i].email, email) == 0) {
            printf("[search] SearchByEmail('%s'): Found %s\n", email, candidates[i].name);
            return 1;
        }
    }
    printf("[search] SearchByEmail('%s'): No match\n", email);
    return 0;
}

int SearchByExperience(int years)
{
    int matches = 0;
    printf("\n[search] SearchByExperience(>= %d years):\n", years);
    for (int i = 0; i < candidateCount; i++) {
        if (candidates[i].experienceYears >= years) {
            printf("  %s - %d year(s)\n", candidates[i].name, candidates[i].experienceYears);
            matches++;
        }
    }
    if (!matches) printf("  No matches\n");
    return matches;
}

int FilterByDegree(const char *degreeKeyword)
{
    int matches = 0;
    printf("\n[search] FilterByDegree('%s'):\n", degreeKeyword);
    for (int i = 0; i < candidateCount; i++) {
        if (StrCaseContains(candidates[i].degree, degreeKeyword)) {
            printf("  %s - %s\n", candidates[i].name, candidates[i].degree);
            matches++;
        }
    }
    if (!matches) printf("  No matches\n");
    return matches;
}
