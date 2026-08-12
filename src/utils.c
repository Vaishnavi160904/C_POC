#include "utils.h"
#include <ctype.h>
#include <dirent.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

void TrimWhitespace(char *str)
{
    if (!str) return;
    int len = (int)strlen(str);
    while (len > 0 && (unsigned char)str[len - 1] <= ' ') {
        str[len - 1] = '\0';
        len--;
    }
    int start = 0;
    while (str[start] != '\0' && (unsigned char)str[start] <= ' ') start++;
    if (start > 0) memmove(str, str + start, strlen(str + start) + 1);
}

void ToLowerCase(char *str)
{
    if (!str) return;
    for (int i = 0; str[i]; i++) str[i] = (char)tolower((unsigned char)str[i]);
}

int FileExists(const char *filepath)
{
    FILE *fp = fopen(filepath, "r");
    if (!fp) return 0;
    fclose(fp);
    return 1;
}

int StrCaseCmp(const char *a, const char *b)
{
    while (*a && *b) {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if (ca != cb) return ca - cb;
        a++; b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

int StrCaseContains(const char *haystack, const char *needle)
{
    if (!haystack || !needle || !*needle) return 0;
    char h[MAX_LINE], n[MAX_WORD_LEN];
    strncpy(h, haystack, sizeof(h) - 1); h[sizeof(h) - 1] = '\0';
    strncpy(n, needle, sizeof(n) - 1); n[sizeof(n) - 1] = '\0';
    ToLowerCase(h);
    ToLowerCase(n);
    return strstr(h, n) != NULL;
}

int IsNumericToken(const char *token)
{
    if (!token || !*token) return 0;
    for (int i = 0; token[i]; i++)
        if (!isdigit((unsigned char)token[i])) return 0;
    return 1;
}

long GetFileSizeFromPath(const char *filepath)
{
    FILE *fp = fopen(filepath, "rb");
    if (!fp) return -1;
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fclose(fp);
    return size;
}

int LoadWordListFile(const char *filepath, char list[][MAX_WORD_LEN], int *count, int maxItems)
{
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        *count = 0;
        return 0;
    }
    char line[MAX_LINE];
    *count = 0;
    while (fgets(line, sizeof(line), fp) && *count < maxItems) {
        TrimWhitespace(line);
        if (strlen(line) == 0) continue;
        strncpy(list[*count], line, MAX_WORD_LEN - 1);
        list[*count][MAX_WORD_LEN - 1] = '\0';
        (*count)++;
    }
    fclose(fp);
    return *count;
}

/* Lowercases, turns every run of non-alphanumeric characters into a single
 * space, and trims the ends. "Node.js" and "Node JS" both become "node js";
 * "C++" becomes "c" (symbols carry no letters to keep - handled as a special
 * case by the caller when needed) */
static void NormalizeForMatch(const char *in, char *out, size_t outSize)
{
    size_t j = 0;
    int lastWasSpace = 1;
    for (size_t i = 0; in[i] != '\0' && j < outSize - 1; i++) {
        unsigned char c = (unsigned char)in[i];
        if (isalnum(c)) {
            out[j++] = (char)tolower(c);
            lastWasSpace = 0;
        } else if (c == '+' || c == '#') {
            /* keep so C++ / C# survive as distinct tokens */
            out[j++] = (char)c;
            lastWasSpace = 0;
        } else {
            if (!lastWasSpace && j < outSize - 1) { out[j++] = ' '; lastWasSpace = 1; }
        }
    }
    while (j > 0 && out[j - 1] == ' ') j--;
    out[j] = '\0';
}

/* normalizedPaddedText must already be NormalizeForMatch()'d and padded with
 * a leading/trailing space, e.g. " some normalized text ". Does a whole
 * word/phrase boundary-safe search. */
static int TextContainsPhrase(const char *normalizedPaddedText, const char *phrase)
{
    char normPhrase[MAX_WORD_LEN];
    NormalizeForMatch(phrase, normPhrase, sizeof(normPhrase));
    if (normPhrase[0] == '\0') return 0;

    char padded[MAX_WORD_LEN + 2];
    snprintf(padded, sizeof(padded), " %s ", normPhrase);
    return strstr(normalizedPaddedText, padded) != NULL;
}

int ExtractKnownSkills(const char *text, char out[][MAX_WORD_LEN], int *outCount, int maxOut)
{
    static char normText[16000];
    NormalizeForMatch(text, normText, sizeof(normText));

    static char padded[16002];
    snprintf(padded, sizeof(padded), " %s ", normText);

    *outCount = 0;
    for (int t = 0; t < techSkillCount && *outCount < maxOut; t++) {
        if (TextContainsPhrase(padded, techSkills[t])) {
            int already = 0;
            for (int k = 0; k < *outCount; k++)
                if (StrCaseCmp(out[k], techSkills[t]) == 0) { already = 1; break; }
            if (!already) {
                strncpy(out[*outCount], techSkills[t], MAX_WORD_LEN - 1);
                out[*outCount][MAX_WORD_LEN - 1] = '\0';
                (*outCount)++;
            }
        }
    }
    return *outCount;
}

char *ConvertPdfToText(const char *pdfPath)
{
    char tmpPath[MAX_PATH_LEN + 8];
    snprintf(tmpPath, sizeof(tmpPath), "%s.tmp.txt", pdfPath);

    char cmd[600];
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "pdftotext -layout \"%s\" \"%s\" 2>NUL", pdfPath, tmpPath);
#else
    snprintf(cmd, sizeof(cmd), "pdftotext -layout \"%s\" \"%s\" 2>/dev/null", pdfPath, tmpPath);
#endif
    int ret = system(cmd);

    FILE *fp = fopen(tmpPath, "rb");
    if (ret != 0 || !fp) {
        printf("[utils] Could not convert '%s' with pdftotext.\n"
               "  Install Poppler for Windows and make sure pdftotext.exe is on your PATH,\n"
               "  or convert the PDF manually (e.g. Word/Adobe 'Save as text') and upload the .txt instead.\n",
               pdfPath);
        if (fp) fclose(fp);
        remove(tmpPath);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (size <= 0) {
        fclose(fp);
        remove(tmpPath);
        return NULL;
    }

    char *buffer = (char *)malloc((size_t)size + 1);
    if (!buffer) { fclose(fp); remove(tmpPath); return NULL; }
    size_t readBytes = fread(buffer, 1, (size_t)size, fp);
    buffer[readBytes] = '\0';
    fclose(fp);
    remove(tmpPath);

    printf("[utils] Converted PDF '%s' to text (%ld bytes)\n", pdfPath, size);
    return buffer;
}

/* Creates a single directory level if missing. Doesn't recursively create
 * multi-level paths, but every path this project writes to (output/reports,
 * output/shortlisted, etc.) already has its parent committed to the repo via
 * .gitkeep, so this only needs to cover the final missing level in practice. */
void EnsureDirectoryExists(const char *path)
{
    if (!path || !*path) return;

    DIR *dir = opendir(path);
    if (dir) {
        closedir(dir);
        return; /* already exists */
    }

#ifdef _WIN32
    if (_mkdir(path) != 0) {
#else
    if (mkdir(path, 0755) != 0) {
#endif
        /* Not fatal - the caller's subsequent fopen() will fail cleanly and
         * report the real problem if this directory truly can't be created. */
    }
}
