#ifndef ANALYZER_H
#define ANALYZER_H

#include "common.h"

int    LoadTechnicalSkills(const char *filepath);
int    CountWordFrequency(const char tokens[][MAX_TOKEN_LEN], int count);
int    FindTopKeywords(int topN);
double KeywordDensity(const char *keyword, int totalWords);
void   DomainDetection(void);
void   ResumeStatistics(void);

#endif
