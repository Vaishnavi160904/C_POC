#ifndef JOB_REQUIREMENT_H
#define JOB_REQUIREMENT_H

#include "common.h"

<<<<<<< HEAD
/* Load a .txt or .pdf requirement and populate the current required skills. */
int UploadJobRequirement(const char *filepath);
=======
int UploadJobRequirement(const char *filepath);
int ReadRequirement(const char *filepath);
int EditRequirement(const char *filepath);
int DeleteRequirement(const char *filepath);
int AddSkillToRequirement(const char *skill);
int RemoveSkillFromRequirement(const char *skill);
>>>>>>> 931690db4b496c0f25d92c94054677714538d9fa

#endif
