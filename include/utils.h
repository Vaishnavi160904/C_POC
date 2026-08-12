#ifndef UTILS_H
#define UTILS_H

#include "common.h"

void TrimWhitespace(char *str);
void ToLowerCase(char *str);
int  FileExists(const char *filepath);
int  StrCaseCmp(const char *a, const char *b);
int  StrCaseContains(const char *haystack, const char *needle);
int  IsNumericToken(const char *token);
long GetFileSizeFromPath(const char *filepath);

/* Generic "one item per line" file loader used by stopwords / skill lists */
int LoadWordListFile(const char *filepath, char list[][MAX_WORD_LEN], int *count, int maxItems);

/* Scans `text` for every phrase in techSkills[] and writes the matches
 * (using techSkills[]'s canonical spelling) into out[]/outCount. Used by
 * both resume skill-extraction and job-requirement PDF parsing. Internally
 * uses phrase-aware matching so multi-word skills like "Linked List" and
 * variants like "Node.js" are recognized correctly, without false positives
 * like "Java" matching inside "JavaScript". */
int ExtractKnownSkills(const char *text, char out[][MAX_WORD_LEN], int *outCount, int maxOut);

/* Converts a PDF to plain text via the `pdftotext` command-line tool
 * (poppler-utils / Poppler for Windows). Returns a malloc'd buffer the
 * caller must free, or NULL if pdftotext isn't available/failed. */
char *ConvertPdfToText(const char *pdfPath);

/* Creates `path` (and, best-effort, its parent) if it doesn't already exist.
 * Safe to call even if the directory already exists. Used before writing
 * reports/shortlist files so a missing output/ folder doesn't silently fail. */
void EnsureDirectoryExists(const char *path);

#endif
