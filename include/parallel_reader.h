#ifndef PARALLEL_READER_H
#define PARALLEL_READER_H

#include "common.h"

typedef struct {
    char filepath[MAX_PATH_LEN];
    char *text;               /* malloc'd raw resume text; caller must free() */
    char *workText;           /* malloc'd cleaned/normalized text; caller must free() */
    char (*tokens)[MAX_TOKEN_LEN]; /* malloc'd stop-word-filtered tokens; caller must free() */
    int rawTokenCount;
    int tokenCount;
    char statusMsg[256];      /* human-readable outcome, printed by the caller afterward */
} ResumeReadResult;

/*
 * Reads/converts and preprocesses every file in filepaths[] to plain text.
 * Preprocessing includes cleaning, normalization, tokenization and stop-word
 * removal. Each resume owns its result buffers, so up to maxThreads workers
 * can safely process different resumes concurrently.
 *
 * The later analysis phase (frequency analysis, categorization, matching,
 * scoring) remains sequential because the legacy analysis modules use shared
 * global state. This gives real parallelism in the I/O + text-preparation
 * stage without introducing data races into the scoring pipeline.
 *
 * Returns the number of worker threads actually used.
 */
int ReadResumesParallel(char filepaths[][MAX_PATH_LEN], int fileCount,
                         ResumeReadResult *results, int maxThreads);

#endif
