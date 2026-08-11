#ifndef SEARCH_H
#define SEARCH_H

#include "common.h"

int SearchByName(const char *name);
int SearchBySkill(const char *skill);
int SearchByEmail(const char *email);
int SearchByExperience(int years);
int FilterByDegree(const char *degreeKeyword);

#endif
