#ifndef JOB_REQUIREMENT_H
#define JOB_REQUIREMENT_H

#include "common.h"

int UploadJobRequirement(const char *filepath);
int ReadRequirement(const char *filepath);
int EditRequirement(const char *filepath);
int DeleteRequirement(const char *filepath);
int AddSkillToRequirement(const char *skill);
int RemoveSkillFromRequirement(const char *skill);

#endif
