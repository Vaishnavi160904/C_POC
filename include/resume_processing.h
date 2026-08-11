#ifndef RESUME_PROCESSING_H
#define RESUME_PROCESSING_H

#include "common.h"

char *ReadResume(const char *filepath);
void  CleanText(char *text);
void  NormalizeWords(char *text);

#endif
