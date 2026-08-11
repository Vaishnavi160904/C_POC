#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include "common.h"

/* The project keeps all working state in global arrays (common.h /
 * globals.c) rather than passing structs around. That's convenient for a
 * small console app, but it means tests MUST reset this shared state before
 * each test runs, or one test's data leaks into the next test and causes
 * confusing, order-dependent failures. Call this at the start of every
 * test (or in a suite's init function) to get a clean slate. */
static inline void ResetTestState(void)
{
    memset(candidates, 0, sizeof(candidates));
    candidateCount = 0;
    currentCandidateIdx = 0;

    memset(jobSkills, 0, sizeof(jobSkills));
    jobSkillCount = 0;

    memset(techSkills, 0, sizeof(techSkills));
    techSkillCount = 0;

    memset(stopWordsList, 0, sizeof(stopWordsList));
    stopWordCount = 0;

    memset(currentKeywords, 0, sizeof(currentKeywords));
    currentKeywordCount = 0;

    memset(categories, 0, sizeof(categories));
    categoryCount = 0;

    memset(resumeFiles, 0, sizeof(resumeFiles));
    resumeFileCount = 0;

    loggedInUser[0] = '\0';
    isLoggedIn = 0;
    shortlistThreshold = 70;
}

/* Loads a fixed, small, deterministic technical-skills list directly into
 * techSkills[] without depending on data/skills.txt (keeps unit tests
 * independent of the real data files, which might change over time). */
static inline void LoadFixtureTechSkills(void)
{
    const char *skills[] = {
        "Python", "Java", "C++", "SQL", "React", "Node.js",
        "MongoDB", "Git", "Docker", "AWS", "JavaScript", "HTML", "CSS"
    };
    techSkillCount = 0;
    for (size_t i = 0; i < sizeof(skills) / sizeof(skills[0]); i++) {
        strncpy(techSkills[techSkillCount], skills[i], MAX_WORD_LEN - 1);
        techSkillCount++;
    }
}

static inline void LoadFixtureJobSkills(void)
{
    const char *skills[] = { "Python", "React", "MongoDB", "Git", "Docker" };
    jobSkillCount = 0;
    for (size_t i = 0; i < sizeof(skills) / sizeof(skills[0]); i++) {
        strncpy(jobSkills[jobSkillCount], skills[i], MAX_WORD_LEN - 1);
        jobSkillCount++;
    }
}

static inline void LoadFixtureStopWords(void)
{
    const char *words[] = { "the", "is", "and", "with", "for", "to", "a", "an", "of" };
    stopWordCount = 0;
    for (size_t i = 0; i < sizeof(words) / sizeof(words[0]); i++) {
        strncpy(stopWordsList[stopWordCount], words[i], MAX_WORD_LEN - 1);
        stopWordCount++;
    }
}

#endif
