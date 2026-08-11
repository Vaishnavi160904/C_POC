#include "job_requirement.h"
#include "utils.h"

#define JOB_REQ_FILE "data/job_requirement.txt"

/* Loads jobSkills[] from any requirement file. .pdf files (e.g. an HR-provided
 * "requirements.pdf" with a skills table) are converted with pdftotext and
 * scanned for every known technical skill (phrase-aware, so "Linked Lists"
 * and "Node.js" are recognized correctly). .txt files are read one skill per
 * line, the simple format used by data/job_requirement.txt. */
static int LoadRequirementFromFile(const char *filepath)
{
    const char *dot = strrchr(filepath, '.');
    int isPdf = dot && StrCaseCmp(dot, ".pdf") == 0;

    if (isPdf) {
        char *text = ConvertPdfToText(filepath);
        if (!text) {
            jobSkillCount = 0;
            return 0;
        }
        ExtractKnownSkills(text, jobSkills, &jobSkillCount, MAX_JOB_SKILLS);
        free(text);
    } else {
        LoadWordListFile(filepath, jobSkills, &jobSkillCount, MAX_JOB_SKILLS);
    }
    return jobSkillCount;
}

int UploadJobRequirement(const char *filepath)
{
    if (!FileExists(filepath)) {
        printf("[job_requirement] Upload failed: '%s' not found\n", filepath);
        return 0;
    }

    int count = LoadRequirementFromFile(filepath);

    /* Persist the parsed skill list to the canonical job_requirement.txt so
     * it can be inspected, reused, or hand-edited afterwards - regardless of
     * whether the source was a .txt list or a .pdf table. */
    FILE *out = fopen(JOB_REQ_FILE, "w");
    if (out) {
        for (int i = 0; i < jobSkillCount; i++) fprintf(out, "%s\n", jobSkills[i]);
        fclose(out);
    }

    printf("[job_requirement] Uploaded '%s': %d required skill(s) loaded\n", filepath, count);
    for (int i = 0; i < jobSkillCount; i++) printf("  - %s\n", jobSkills[i]);
    return count;
}

int ReadRequirement(const char *filepath)
{
    int count = LoadRequirementFromFile(filepath);
    printf("[job_requirement] Required Skills (%d):\n", count);
    for (int i = 0; i < jobSkillCount; i++) printf("  - %s\n", jobSkills[i]);
    return count;
}

int EditRequirement(const char *filepath)
{
    int count = LoadRequirementFromFile(filepath);
    printf("[job_requirement] Reloaded requirement file '%s' (%d skills)\n", filepath, count);
    return count;
}

int DeleteRequirement(const char *filepath)
{
    if (remove(filepath) == 0) {
        jobSkillCount = 0;
        printf("[job_requirement] Deleted requirement file '%s'\n", filepath);
        return 1;
    }
    printf("[job_requirement] Delete failed: '%s' not found\n", filepath);
    return 0;
}

int AddSkillToRequirement(const char *skill)
{
    if (jobSkillCount >= MAX_JOB_SKILLS) return 0;
    for (int i = 0; i < jobSkillCount; i++)
        if (StrCaseCmp(jobSkills[i], skill) == 0) return 0; /* already present */

    strncpy(jobSkills[jobSkillCount], skill, MAX_WORD_LEN - 1);
    jobSkills[jobSkillCount][MAX_WORD_LEN - 1] = '\0';
    jobSkillCount++;

    FILE *fp = fopen(JOB_REQ_FILE, "a");
    if (fp) { fprintf(fp, "%s\n", skill); fclose(fp); }
    printf("[job_requirement] Added skill '%s'\n", skill);
    return 1;
}

int RemoveSkillFromRequirement(const char *skill)
{
    int found = -1;
    for (int i = 0; i < jobSkillCount; i++)
        if (StrCaseCmp(jobSkills[i], skill) == 0) { found = i; break; }
    if (found < 0) return 0;

    for (int i = found; i < jobSkillCount - 1; i++)
        strcpy(jobSkills[i], jobSkills[i + 1]);
    jobSkillCount--;

    FILE *fp = fopen(JOB_REQ_FILE, "w");
    if (fp) {
        for (int i = 0; i < jobSkillCount; i++) fprintf(fp, "%s\n", jobSkills[i]);
        fclose(fp);
    }
    printf("[job_requirement] Removed skill '%s'\n", skill);
    return 1;
}
