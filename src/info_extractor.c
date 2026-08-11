#include "info_extractor.h"
#include "utils.h"
#include <ctype.h>

/* Resumes format section headers wildly differently ("Skills", "PROJECTS",
 * "TECHNOLOGIES AND SKILLS", "COMPETITIVE PROGRAMMING", ...). Rather than
 * maintaining a fixed keyword list, treat any short, mostly-uppercase,
 * non-bullet line as a new section header - this generalizes across
 * resume styles instead of only recognizing a handful of exact headers. */
static int IsLikelySectionHeader(const char *line)
{
    int len = (int)strlen(line);
    if (len == 0 || len > 45) return 0;
    if (line[0] == '-' || line[0] == '*' || line[0] == '.') return 0;

    int letters = 0, upper = 0;
    for (int i = 0; i < len; i++) {
        unsigned char ch = (unsigned char)line[i];
        if (isalpha(ch)) {
            letters++;
            if (isupper(ch)) upper++;
        }
    }
    if (letters < 3) return 0;
    return upper == letters; /* every letter in the line is uppercase */
}

/* pdftotext -layout pads columns with runs of spaces to preserve visual
 * layout (e.g. "Chennai Institute Of Technology          CGPA: 8.6").
 * Collapse those runs down to a single space for cleaner extracted fields. */
static void CollapseInternalSpaces(char *str)
{
    int j = 0;
    int lastWasSpace = 0;
    for (int i = 0; str[i]; i++) {
        if (str[i] == ' ' || str[i] == '\t') {
            if (!lastWasSpace) str[j++] = ' ';
            lastWasSpace = 1;
        } else {
            str[j++] = str[i];
            lastWasSpace = 0;
        }
    }
    str[j] = '\0';
}

/* First non-empty line that doesn't look like an email/phone/section header
 * is taken as the candidate's name (common resume convention). */
int ExtractName(const char *text, char *outName)
{
    char copy[8000];
    strncpy(copy, text, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    const char *line = strtok(copy, "\n");
    while (line != NULL) {
        char trimmed[MAX_LINE];
        strncpy(trimmed, line, sizeof(trimmed) - 1);
        trimmed[sizeof(trimmed) - 1] = '\0';
        TrimWhitespace(trimmed);

        if (strlen(trimmed) > 1 && strlen(trimmed) < 60 &&
            !strchr(trimmed, '@') && !isdigit((unsigned char)trimmed[0])) {
            strncpy(outName, trimmed, 99);
            outName[99] = '\0';
            printf("[info_extractor] ExtractName: %s\n", outName);
            return 1;
        }
        line = strtok(NULL, "\n");
    }

    strcpy(outName, "Unknown Candidate");
    printf("[info_extractor] ExtractName: could not confidently detect a name\n");
    return 0;
}

int ExtractEmail(const char *text, char *outEmail)
{
    char copy[8000];
    strncpy(copy, text, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    const char *token = strtok(copy, " \t\r\n");
    while (token != NULL) {
        char *at = strchr(token, '@');
        if (at && strchr(at, '.')) {
            /* strip trailing punctuation like , or ) */
            char clean[100];
            int len = 0;
            for (int i = 0; token[i] && len < 99; i++) {
                if (isalnum((unsigned char)token[i]) || strchr("@._-+", token[i]))
                    clean[len++] = token[i];
            }
            clean[len] = '\0';
            strncpy(outEmail, clean, 99);
            outEmail[99] = '\0';
            printf("[info_extractor] ExtractEmail: %s\n", outEmail);
            return 1;
        }
        token = strtok(NULL, " \t\r\n");
    }

    outEmail[0] = '\0';
    printf("[info_extractor] ExtractEmail: not found\n");
    return 0;
}

int ExtractPhone(const char *text, char *outPhone)
{
    char copy[8000];
    strncpy(copy, text, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    const char *token = strtok(copy, " \t\r\n");
    while (token != NULL) {
        char digitsOnly[30];
        int len = 0;
        for (int i = 0; token[i] && len < 29; i++)
            if (isdigit((unsigned char)token[i]) || (token[i] == '+' && i == 0))
                digitsOnly[len++] = token[i];
        digitsOnly[len] = '\0';

        /* Validate the digits themselves (defensive check, not just a count)
         * before accepting this token as a phone number. */
        int digitStart = (digitsOnly[0] == '+') ? 1 : 0;
        if (IsNumericToken(digitsOnly + digitStart)) {
            int digitCount = (int)strlen(digitsOnly) - digitStart;
            if (digitCount >= 10 && digitCount <= 13) {
                strncpy(outPhone, digitsOnly, 29);
                outPhone[29] = '\0';
                printf("[info_extractor] ExtractPhone: %s\n", outPhone);
                return 1;
            }
        }
        token = strtok(NULL, " \t\r\n");
    }

    outPhone[0] = '\0';
    printf("[info_extractor] ExtractPhone: not found\n");
    return 0;
}

/* Compares the resume text against the known techSkills[] list using
 * phrase-aware matching (handles multi-word skills like "Linked List" and
 * avoids false positives like "Java" matching inside "JavaScript"). */
int ExtractSkills(const char *text)
{
    Candidate *c = CURRENT;
    ExtractKnownSkills(text, c->skills, &c->skillCount, MAX_CAND_SKILLS);
    printf("[info_extractor] ExtractSkills: %d matched skill(s)\n", c->skillCount);
    return c->skillCount;
}

/* Looks for a "Projects" section header and captures the following non-empty
 * lines (until the next apparent section header) as project titles. */
int ExtractProjects(const char *text)
{
    Candidate *c = CURRENT;
    c->projectCount = 0;

    char copy[8000];
    strncpy(copy, text, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    const char *line = strtok(copy, "\n");
    int inSection = 0;
    while (line != NULL) {
        char trimmed[MAX_LINE];
        strncpy(trimmed, line, sizeof(trimmed) - 1);
        trimmed[sizeof(trimmed) - 1] = '\0';
        TrimWhitespace(trimmed);

        if (StrCaseCmp(trimmed, "Projects") == 0 || StrCaseCmp(trimmed, "Project") == 0) {
            inSection = 1;
            line = strtok(NULL, "\n");
            continue;
        }

        if (inSection) {
            if (strlen(trimmed) == 0) { line = strtok(NULL, "\n"); continue; }
            if (IsLikelySectionHeader(trimmed)) break;
            if (trimmed[0] == '-' || trimmed[0] == '*') { line = strtok(NULL, "\n"); continue; } /* bullet description, not a title */

            if (c->projectCount < MAX_PROJECTS) {
                strncpy(c->projects[c->projectCount], trimmed, MAX_PROJECT_LEN - 1);
                c->projects[c->projectCount][MAX_PROJECT_LEN - 1] = '\0';
                c->projectCount++;
            }
        }
        line = strtok(NULL, "\n");
    }

    printf("[info_extractor] ExtractProjects: %d project(s) found\n", c->projectCount);
    return c->projectCount;
}

/* Same section-capture heuristic as ExtractProjects, but for "Certifications" */
int ExtractCertifications(const char *text)
{
    Candidate *c = CURRENT;
    c->certCount = 0;

    char copy[8000];
    strncpy(copy, text, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    const char *line = strtok(copy, "\n");
    int inSection = 0;
    while (line != NULL) {
        char trimmed[MAX_LINE];
        strncpy(trimmed, line, sizeof(trimmed) - 1);
        trimmed[sizeof(trimmed) - 1] = '\0';
        TrimWhitespace(trimmed);

        if (StrCaseCmp(trimmed, "Certifications") == 0 || StrCaseCmp(trimmed, "Certification") == 0) {
            inSection = 1;
            line = strtok(NULL, "\n");
            continue;
        }

        if (inSection) {
            if (strlen(trimmed) == 0) { line = strtok(NULL, "\n"); continue; }
            if (IsLikelySectionHeader(trimmed)) break;

            if (c->certCount < MAX_CERTS) {
                strncpy(c->certifications[c->certCount], trimmed, MAX_CERT_LEN - 1);
                c->certifications[c->certCount][MAX_CERT_LEN - 1] = '\0';
                c->certCount++;
            }
        }
        line = strtok(NULL, "\n");
    }

    printf("[info_extractor] ExtractCertifications: %d certification(s) found\n", c->certCount);
    return c->certCount;
}

int ExtractEducation(const char *text)
{
    Candidate *c = CURRENT;
    static const char *degreeKeywords[] = {"B.E", "B.Tech", "BE", "BTech", "M.Tech", "MTech",
                                            "Bachelor", "Master", "MCA", "BCA", "MSc", "BSc", "PhD"};
    c->degree[0] = '\0';
    c->college[0] = '\0';

    char copy[8000];
    strncpy(copy, text, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    const char *line = strtok(copy, "\n");
    while (line != NULL) {
        char trimmed[MAX_LINE];
        strncpy(trimmed, line, sizeof(trimmed) - 1);
        trimmed[sizeof(trimmed) - 1] = '\0';
        TrimWhitespace(trimmed);
        CollapseInternalSpaces(trimmed);

        if (!c->degree[0]) {
            for (size_t k = 0; k < sizeof(degreeKeywords) / sizeof(degreeKeywords[0]); k++) {
                if (StrCaseContains(trimmed, degreeKeywords[k])) {
                    strncpy(c->degree, trimmed, sizeof(c->degree) - 1);
                    break;
                }
            }
        }
        if (!c->college[0] &&
            (StrCaseContains(trimmed, "College") || StrCaseContains(trimmed, "University") ||
             StrCaseContains(trimmed, "Institute"))) {
            strncpy(c->college, trimmed, sizeof(c->college) - 1);
        }
        line = strtok(NULL, "\n");
    }

    printf("[info_extractor] ExtractEducation: degree='%s' college='%s'\n", c->degree, c->college);
    return c->degree[0] != '\0';
}

/* Looks for patterns like "2 years", "3+ years experience" */
int ExtractExperience(const char *text)
{
    Candidate *c = CURRENT;
    c->experienceYears = 0;

    char copy[8000];
    strncpy(copy, text, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    const char *token = strtok(copy, " \t\r\n");
    char prevToken[MAX_WORD_LEN] = "";
    while (token != NULL) {
        if (StrCaseContains(token, "year")) {
            int years = 0;
            if (sscanf(prevToken, "%d", &years) == 1 && years > 0 && years < 60) {
                c->experienceYears = years;
                break;
            }
        }
        strncpy(prevToken, token, MAX_WORD_LEN - 1);
        prevToken[MAX_WORD_LEN - 1] = '\0';
        token = strtok(NULL, " \t\r\n");
    }

    printf("[info_extractor] ExtractExperience: %d year(s)\n", c->experienceYears);
    return c->experienceYears;
}
