#include "skill_match.h"
#include "utils.h"

int MatchSkills(void)
{
    Candidate *c = CURRENT;
    c->matchedSkillCount = 0;

    for (int j = 0; j < jobSkillCount; j++) {
        for (int s = 0; s < c->skillCount; s++) {
            if (StrCaseCmp(jobSkills[j], c->skills[s]) == 0) {
                if (c->matchedSkillCount < MAX_JOB_SKILLS) {
                    strncpy(c->matchedSkills[c->matchedSkillCount], jobSkills[j], MAX_WORD_LEN - 1);
                    c->matchedSkills[c->matchedSkillCount][MAX_WORD_LEN - 1] = '\0';
                    c->matchedSkillCount++;
                }
                break;
            }
        }
    }

    printf("\n[skill_match] Matched Skills (%d/%d):\n", c->matchedSkillCount, jobSkillCount);
    for (int i = 0; i < c->matchedSkillCount; i++) printf("  - %s\n", c->matchedSkills[i]);
    return c->matchedSkillCount;
}

int MissingSkills(void)
{
    Candidate *c = CURRENT;
    c->missingSkillCount = 0;

    for (int j = 0; j < jobSkillCount; j++) {
        int found = 0;
        for (int m = 0; m < c->matchedSkillCount; m++)
            if (StrCaseCmp(jobSkills[j], c->matchedSkills[m]) == 0) { found = 1; break; }
        if (!found && c->missingSkillCount < MAX_JOB_SKILLS) {
            strncpy(c->missingSkills[c->missingSkillCount], jobSkills[j], MAX_WORD_LEN - 1);
            c->missingSkills[c->missingSkillCount][MAX_WORD_LEN - 1] = '\0';
            c->missingSkillCount++;
        }
    }

    printf("[skill_match] Missing Skills (%d):\n", c->missingSkillCount);
    for (int i = 0; i < c->missingSkillCount; i++) printf("  - %s\n", c->missingSkills[i]);
    return c->missingSkillCount;
}

double CalculateMatchPercentage(int matched, int total)
{
    double pct = (total > 0) ? (matched * 100.0 / total) : 0.0;
    CURRENT->matchPercentage = pct;
    printf("[skill_match] Skill Match: %.1f%%\n", pct);
    return pct;
}
