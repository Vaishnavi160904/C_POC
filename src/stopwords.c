#include "stopwords.h"
#include "utils.h"

int LoadStopWords(const char *filepath)
{
    int n = LoadWordListFile(filepath, stopWordsList, &stopWordCount, MAX_STOPWORDS);
    printf("[stopwords] Loaded %d stop words from '%s'\n", n, filepath);
    return n;
}

/* Only used internally by RemoveStopWords() - not part of the public module API. */
static int IsStopWord(const char *word)
{
    if (strlen(word) < 2) return 1; /* drop single characters too */
    for (int i = 0; i < stopWordCount; i++)
        if (StrCaseCmp(stopWordsList[i], word) == 0) return 1;
    return 0;
}

/* Compacts tokens[] in place, keeping only non-stop-words. Returns new count. */
int RemoveStopWords(char tokens[][MAX_TOKEN_LEN], int count)
{
    int kept = 0;
    for (int i = 0; i < count; i++) {
        if (!IsStopWord(tokens[i])) {
            if (kept != i) strcpy(tokens[kept], tokens[i]);
            kept++;
        }
    }
    printf("[stopwords] RemoveStopWords: %d -> %d tokens remain\n", count, kept);
    return kept;
}
