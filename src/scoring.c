#include "scoring.h"
#include "logger.h"

/*
 * Weighting (out of 100 total, matching the project's example report):
 *   Skill Match     -> up to 50
 *   Projects        -> up to 20
 *   Experience      -> up to 15
 *   Education       -> up to 10
 *   Certifications  -> up to 5
 */

int CalculateSkillScore(void)
{
    Candidate *c = CURRENT;
    c->skillScore = (int)((c->matchPercentage / 100.0) * 50.0 + 0.5);
    printf("[scoring] Skill Score: %d/50\n", c->skillScore);
    return c->skillScore;
}

int CalculateExperienceScore(void)
{
    Candidate *c = CURRENT;
    int score = c->experienceYears * 3;
    if (score > 15) score = 15;
    c->experienceScore = score;
    printf("[scoring] Experience Score: %d/15\n", c->experienceScore);
    return c->experienceScore;
}

int CalculateEducationScore(void)
{
    Candidate *c = CURRENT;
    c->educationScore = (c->degree[0] != '\0') ? 10 : 4;
    printf("[scoring] Education Score: %d/10\n", c->educationScore);
    return c->educationScore;
}

int CalculateProjectScore(void)
{
    Candidate *c = CURRENT;
    int score = c->projectCount * 7;
    if (score > 20) score = 20;
    c->projectScore = score;
    printf("[scoring] Project Score: %d/20\n", c->projectScore);
    return c->projectScore;
}

int CalculateCertificationScore(void)
{
    Candidate *c = CURRENT;
    int score = c->certCount * 2;
    if (score > 5) score = 5;
    c->certScore = score;
    printf("[scoring] Certification Score: %d/5\n", c->certScore);
    return c->certScore;
}

int GenerateFinalScore(void)
{
    Candidate *c = CURRENT;
    c->finalScore = c->skillScore + c->experienceScore + c->educationScore +
                     c->projectScore + c->certScore;
    if (c->finalScore > 100) c->finalScore = 100;
    printf("[scoring] FINAL SCORE: %d/100\n", c->finalScore);
    printf("[scoring] Breakdown: Skill %d/50 + Experience %d/15 + Education %d/10 + Projects %d/20 + Certifications %d/5\n",
           c->skillScore, c->experienceScore, c->educationScore, c->projectScore, c->certScore);
    char msg[256];
    snprintf(msg, sizeof(msg),
             "Score explanation: Skill=%d/50, Experience=%d/15, Education=%d/10, Projects=%d/20, Certifications=%d/5, Final=%d/100",
             c->skillScore, c->experienceScore, c->educationScore, c->projectScore, c->certScore, c->finalScore);
    LogInfo("SCORING", msg);
    return c->finalScore;
}
