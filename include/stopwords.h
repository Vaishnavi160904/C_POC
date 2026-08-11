#ifndef STOPWORDS_H
#define STOPWORDS_H

#include "common.h"

int LoadStopWords(const char *filepath);
int RemoveStopWords(char tokens[][MAX_TOKEN_LEN], int count);

#endif
