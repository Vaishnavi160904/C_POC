#ifndef INFO_EXTRACTOR_H
#define INFO_EXTRACTOR_H

#include "common.h"

int ExtractName(const char *text, char *outName);
int ExtractEmail(const char *text, char *outEmail);
int ExtractPhone(const char *text, char *outPhone);
int ExtractSkills(const char *text);
int ExtractProjects(const char *text);
int ExtractCertifications(const char *text);
int ExtractEducation(const char *text);
int ExtractExperience(const char *text);

#endif
