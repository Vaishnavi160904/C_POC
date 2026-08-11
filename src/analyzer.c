#include "analyzer.h"
#include "utils.h"

int LoadTechnicalSkills(const char *filepath)
{
    int n = LoadWordListFile(filepath, techSkills, &techSkillCount, MAX_TECH_SKILLS);
    printf("[analyzer] Loaded %d known technical skills from '%s'\n", n, filepath);
    return n;
}

/* Builds currentKeywords[] : a word -> frequency table for the tokens given. */
int CountWordFrequency(const char tokens[][MAX_TOKEN_LEN], int count)
{
    currentKeywordCount = 0;
    for (int i = 0; i < count; i++) {
        int found = -1;
        for (int j = 0; j < currentKeywordCount; j++) {
            if (strcmp(currentKeywords[j].word, tokens[i]) == 0) { found = j; break; }
        }
        if (found >= 0) {
            currentKeywords[found].count++;
        } else if (currentKeywordCount < MAX_KEYWORDS) {
            strncpy(currentKeywords[currentKeywordCount].word, tokens[i], MAX_WORD_LEN - 1);
            currentKeywords[currentKeywordCount].word[MAX_WORD_LEN - 1] = '\0';
            currentKeywords[currentKeywordCount].count = 1;
            currentKeywordCount++;
        }
    }
    printf("[analyzer] CountWordFrequency: %d unique words\n", currentKeywordCount);
    return currentKeywordCount;
}

/* Sorts a scratch copy of currentKeywords[] by frequency (desc, selection sort)
 * and prints/stores the top N. Only keeps entries that are recognized technical
 * skills for the candidate's topSkills[] list (matches project spec: "Top Skills"). */
int FindTopKeywords(int topN)
{
    KeywordFreq sorted[MAX_KEYWORDS];
    memcpy(sorted, currentKeywords, sizeof(KeywordFreq) * currentKeywordCount);

    for (int i = 0; i < currentKeywordCount - 1; i++) {
        int maxIdx = i;
        for (int j = i + 1; j < currentKeywordCount; j++)
            if (sorted[j].count > sorted[maxIdx].count) maxIdx = j;
        if (maxIdx != i) {
            KeywordFreq tmp = sorted[i];
            sorted[i] = sorted[maxIdx];
            sorted[maxIdx] = tmp;
        }
    }

    printf("\n[analyzer] Top Word Frequencies:\n");
    int shown = topN < currentKeywordCount ? topN : currentKeywordCount;
    for (int i = 0; i < shown; i++)
        printf("  %-20s %d\n", sorted[i].word, sorted[i].count);

    /* Build the candidate's "Top Skills" list = highest-frequency *known* tech skills */
    Candidate *c = CURRENT;
    c->topSkillCount = 0;
    for (int i = 0; i < currentKeywordCount && c->topSkillCount < 10; i++) {
        for (int t = 0; t < techSkillCount; t++) {
            if (StrCaseCmp(techSkills[t], sorted[i].word) == 0) {
                strncpy(c->topSkills[c->topSkillCount], techSkills[t], MAX_WORD_LEN - 1);
                c->topSkills[c->topSkillCount][MAX_WORD_LEN - 1] = '\0';
                c->topSkillCount++;
                break;
            }
        }
    }

    return shown;
}

double KeywordDensity(const char *keyword, int totalWords)
{
    if (totalWords <= 0) return 0.0;
    int freq = 0;
    for (int i = 0; i < currentKeywordCount; i++)
        if (StrCaseCmp(currentKeywords[i].word, keyword) == 0) { freq = currentKeywords[i].count; break; }

    double density = (freq * 100.0) / totalWords;
    printf("[analyzer] KeywordDensity('%s') = %.2f%% (%d / %d words)\n", keyword, density, freq, totalWords);
    return density;
}

/* Picks the category with the most keyword matches as the primary domain,
 * and the runner-up as the secondary domain. Relies on CategorizeKeywords()
 * having already populated categories[].matchCount for this resume. */
void DomainDetection(void)
{
    int best = -1, second = -1;
    for (int c = 0; c < categoryCount; c++) {
        if (best == -1 || categories[c].matchCount > categories[best].matchCount) {
            second = best;
            best = c;
        } else if (second == -1 || categories[c].matchCount > categories[second].matchCount) {
            second = c;
        }
    }

    Candidate *cand = CURRENT;
    if (best != -1 && categories[best].matchCount > 0)
        strncpy(cand->primaryDomain, categories[best].name, sizeof(cand->primaryDomain) - 1);
    else
        strncpy(cand->primaryDomain, "General / Unclassified", sizeof(cand->primaryDomain) - 1);

    if (second != -1 && categories[second].matchCount > 0)
        strncpy(cand->secondaryDomain, categories[second].name, sizeof(cand->secondaryDomain) - 1);
    else
        cand->secondaryDomain[0] = '\0';

    printf("[analyzer] Primary Domain: %s\n", cand->primaryDomain);
    if (cand->secondaryDomain[0])
        printf("[analyzer] Secondary Domain: %s\n", cand->secondaryDomain);
}

void ResumeStatistics(void)
{
    Candidate *c = CURRENT;

    int technicalCount = 0;
    for (int i = 0; i < currentKeywordCount; i++)
        for (int t = 0; t < techSkillCount; t++)
            if (StrCaseCmp(techSkills[t], currentKeywords[i].word) == 0) { technicalCount++; break; }

    c->uniqueKeywords = currentKeywordCount;
    c->technicalKeywordCount = technicalCount;

    printf("\n[analyzer] Resume Statistics:\n");
    printf("  Total Words           : %d\n", c->totalWords);
    printf("  Unique Keywords       : %d\n", c->uniqueKeywords);
    printf("  Technical Keywords    : %d\n", c->technicalKeywordCount);
    printf("  Projects Found        : %d\n", c->projectCount);
    printf("  Certifications Found  : %d\n", c->certCount);
    printf("  Experience (years)    : %d\n", c->experienceYears);
}
