#include "tokenizer.h"
#include <ctype.h>

/*
 * Thread-safe tokenizer.
 *
 * The previous implementation used strtok(), whose internal global cursor
 * is shared between threads. Resume preprocessing now runs concurrently,
 * so a manual scanner is used instead. This keeps each call completely
 * independent and safe for pthread workers.
 */
int Tokenize(const char *text, char tokens[][MAX_TOKEN_LEN], int maxTokens)
{
    if (!text || !tokens || maxTokens <= 0) return 0;

    int count = 0;
    size_t i = 0;

    while (text[i] != '\0' && count < maxTokens) {
        while (text[i] != '\0' && isspace((unsigned char)text[i]))
            i++;

        if (text[i] == '\0')
            break;

        int j = 0;
        while (text[i] != '\0' && !isspace((unsigned char)text[i])) {
            if (j < MAX_TOKEN_LEN - 1)
                tokens[count][j++] = text[i];
            i++;
        }
        tokens[count][j] = '\0';

        if (j > 0)
            count++;
    }

    printf("[tokenizer] Tokenize: %d tokens generated\n", count);
    return count;
}
