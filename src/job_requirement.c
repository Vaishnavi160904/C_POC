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

