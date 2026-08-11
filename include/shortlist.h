#ifndef SHORTLIST_H
#define SHORTLIST_H

#include "common.h"

/* Legacy/manual top-N selection retained for backward compatibility. */
int SelectTopCandidates(int topN);

/* Automatically shortlist every candidate whose final score is >= threshold. */
int ShortlistByScoreThreshold(int threshold);

/* Set/get the HR shortlisting threshold (0-100). */
int SetShortlistThreshold(int threshold);
int GetShortlistThreshold(void);

/* Write the current automatic shortlist to disk. */
int GenerateShortlist(void);

#endif
