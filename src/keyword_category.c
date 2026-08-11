#include "keyword_category.h"
#include "utils.h"

/*
 * categories.txt format: blocks separated by blank lines.
 * First line of a block = category name, following lines = skills in it.
 *
 *   Frontend
 *   React
 *   HTML
 *   CSS
 *
 *   Backend
 *   NodeJS
 *   ...
 */
int LoadCategories(const char *filepath)
{
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        printf("[keyword_category] LoadCategories failed: cannot open '%s'\n", filepath);
        return 0;
    }

    categoryCount = 0;
    char line[MAX_LINE];
    int expectingNewCategory = 1;

    while (fgets(line, sizeof(line), fp)) {
        TrimWhitespace(line);
        if (strlen(line) == 0) {
            expectingNewCategory = 1;
            continue;
        }
        if (expectingNewCategory) {
            if (categoryCount >= MAX_CATEGORIES) break;
            strncpy(categories[categoryCount].name, line, MAX_WORD_LEN - 1);
            categories[categoryCount].name[MAX_WORD_LEN - 1] = '\0';
            categories[categoryCount].skillCount = 0;
            categories[categoryCount].matchCount = 0;
            categoryCount++;
            expectingNewCategory = 0;
        } else {
            Category *cat = &categories[categoryCount - 1];
            if (cat->skillCount < MAX_SKILLS_PER_CATEGORY) {
                strncpy(cat->skills[cat->skillCount], line, MAX_WORD_LEN - 1);
                cat->skills[cat->skillCount][MAX_WORD_LEN - 1] = '\0';
                cat->skillCount++;
            }
        }
    }
    fclose(fp);
    printf("[keyword_category] Loaded %d categories from '%s'\n", categoryCount, filepath);
    return categoryCount;
}

/* Cross-references currentKeywords[] against each category's skill list */
int CategorizeKeywords(void)
{
    for (int c = 0; c < categoryCount; c++) categories[c].matchCount = 0;

    printf("\n[keyword_category] Keyword Categorization:\n");
    for (int c = 0; c < categoryCount; c++) {
        printf("  %s:", categories[c].name);
        int any = 0;
        for (int s = 0; s < categories[c].skillCount; s++) {
            for (int k = 0; k < currentKeywordCount; k++) {
                if (StrCaseCmp(categories[c].skills[s], currentKeywords[k].word) == 0) {
                    printf(" %s", categories[c].skills[s]);
                    categories[c].matchCount++;
                    any = 1;
                    break;
                }
            }
        }
        if (!any) printf(" (none found)");
        printf("\n");
    }
    return categoryCount;
}
