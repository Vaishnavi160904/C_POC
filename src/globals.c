#include "common.h"

Candidate candidates[MAX_CANDIDATES];
int candidateCount = 0;
int currentCandidateIdx = 0;

char jobSkills[MAX_JOB_SKILLS][MAX_WORD_LEN];
int  jobSkillCount = 0;

char techSkills[MAX_TECH_SKILLS][MAX_WORD_LEN];
int  techSkillCount = 0;

char stopWordsList[MAX_STOPWORDS][MAX_WORD_LEN];
int  stopWordCount = 0;

KeywordFreq currentKeywords[MAX_KEYWORDS];
int currentKeywordCount = 0;

Category categories[MAX_CATEGORIES];
int categoryCount = 0;

char resumeFiles[MAX_RESUME_FILES][MAX_PATH_LEN];
int  resumeFileCount = 0;

char loggedInUser[MAX_EMAIL_LEN] = "";
int  isLoggedIn = 0;
int  shortlistThreshold = 70;
