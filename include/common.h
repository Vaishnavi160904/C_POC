#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------- Size limits -------------------- */
#define MAX_CANDIDATES        100
#define MAX_WORD_LEN           50
#define MAX_LINE              1024
#define MAX_TOKENS            3000
#define MAX_TOKEN_LEN          MAX_WORD_LEN
#define MAX_KEYWORDS           500
#define MAX_STOPWORDS           300
#define MAX_JOB_SKILLS           30
#define MAX_TECH_SKILLS        1000
#define MAX_CAND_SKILLS          50
#define MAX_PROJECTS             10
#define MAX_PROJECT_LEN         100
#define MAX_CERTS                10
#define MAX_CERT_LEN            100
#define MAX_CATEGORIES           26
#define MAX_SKILLS_PER_CATEGORY  50
#define MAX_RESUME_FILES         50
#define DEFAULT_RESUME_WORKER_THREADS 8
#define MAX_PATH_LEN            260
#define MAX_RESUME_SIZE_BYTES   (5L * 1024L * 1024L) /* 5 MB */
#define MAX_EMAIL_LEN           256 /* RFC-style maximum email address length + terminator */
#define MIN_PASSWORD_LEN        8
#define AUTH_SALT_BYTES         16

/* -------------------- Core structures -------------------- */

typedef struct {
    char word[MAX_WORD_LEN];
    int  count;
} KeywordFreq;

typedef struct {
    char name[MAX_WORD_LEN];
    char skills[MAX_SKILLS_PER_CATEGORY][MAX_WORD_LEN];
    int  skillCount;
    int  matchCount;   /* how many of this category's skills appear in current resume */
} Category;

typedef struct {
    char filename[MAX_PATH_LEN];

    char name[100];
    char email[MAX_EMAIL_LEN];
    char phone[30];
    char degree[100];
    char college[150];
    int  experienceYears;

    char skills[MAX_CAND_SKILLS][MAX_WORD_LEN];
    int  skillCount;

    char projects[MAX_PROJECTS][MAX_PROJECT_LEN];
    int  projectCount;

    char certifications[MAX_CERTS][MAX_CERT_LEN];
    int  certCount;

    int  totalWords;
    int  uniqueKeywords;
    int  technicalKeywordCount;

    char topSkills[10][MAX_WORD_LEN];
    int  topSkillCount;

    char primaryDomain[50];
    char secondaryDomain[50];

    char matchedSkills[MAX_JOB_SKILLS][MAX_WORD_LEN];
    int  matchedSkillCount;
    char missingSkills[MAX_JOB_SKILLS][MAX_WORD_LEN];
    int  missingSkillCount;
    double matchPercentage;

    int skillScore;
    int experienceScore;
    int educationScore;
    int projectScore;
    int certScore;
    int finalScore;

    int rank;
    int shortlisted;
} Candidate;

/* -------------------- Global state (defined in globals.c) -------------------- */

extern Candidate candidates[MAX_CANDIDATES];
extern int candidateCount;      /* number of candidates fully processed   */
extern int currentCandidateIdx; /* index currently being built/processed  */

extern char jobSkills[MAX_JOB_SKILLS][MAX_WORD_LEN];
extern int  jobSkillCount;

extern char techSkills[MAX_TECH_SKILLS][MAX_WORD_LEN];
extern int  techSkillCount;

extern char stopWordsList[MAX_STOPWORDS][MAX_WORD_LEN];
extern int  stopWordCount;

extern KeywordFreq currentKeywords[MAX_KEYWORDS];
extern int currentKeywordCount;

extern Category categories[MAX_CATEGORIES];
extern int categoryCount;

extern char resumeFiles[MAX_RESUME_FILES][MAX_PATH_LEN];
extern int  resumeFileCount;

extern char loggedInUser[MAX_EMAIL_LEN];
extern int  isLoggedIn;

/* HR-configurable automatic shortlisting threshold (0-100). */
extern int  shortlistThreshold;

/* current candidate helper */
#define CURRENT (&candidates[currentCandidateIdx])

#endif
