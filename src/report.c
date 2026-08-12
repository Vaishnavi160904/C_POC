#include "report.h"
#include "utils.h"
#include "logger.h"

int GenerateCSV(const char *outFile)
{
    EnsureDirectoryExists("output");
    EnsureDirectoryExists("output/reports");

    FILE *fp = fopen(outFile, "w");
    if (!fp) {
        printf("[report] GenerateCSV failed: cannot open '%s'\n", outFile);
        LogError("REPORT", "CSV report file could not be opened");
        return 0;
    }

    fprintf(fp, "Automatic Shortlist Threshold, %d%%\n", shortlistThreshold);
    fprintf(fp, "Rule,Final Score >= threshold is SHORTLISTED\n\n");
    fprintf(fp, "Rank,Name,Email,Phone,Experience(yrs),MatchedSkills,MissingSkills,MatchPercent,SkillScore,ExperienceScore,EducationScore,ProjectScore,CertificationScore,FinalScore,Shortlisted\n");
    for (int i = 0; i < candidateCount; i++) {
        Candidate *c = &candidates[i];
        fprintf(fp, "%d,%s,%s,%s,%d,%d,%d,%.1f,%d,%d,%d,%d,%d,%d,%s\n",
                c->rank, c->name, c->email, c->phone, c->experienceYears,
                c->matchedSkillCount, c->missingSkillCount, c->matchPercentage,
                c->skillScore, c->experienceScore, c->educationScore, c->projectScore, c->certScore,
                c->finalScore, c->shortlisted ? "Yes" : "No");
    }
    fclose(fp);
    printf("[report] GenerateCSV: wrote %d row(s) to '%s'\n", candidateCount, outFile);
    LogInfo("REPORT", "CSV report generated with explainable score components");
    return candidateCount;
}

int GenerateTXT(const char *outFile)
{
    EnsureDirectoryExists("output");
    EnsureDirectoryExists("output/reports");

    FILE *fp = fopen(outFile, "w");
    if (!fp) {
        printf("[report] GenerateTXT failed: cannot open '%s'\n", outFile);
        LogError("REPORT", "TXT report file could not be opened");
        return 0;
    }

    fprintf(fp, "HR RESUME SCREENING - CANDIDATE REPORT\n");
    fprintf(fp, "Automatic Shortlist Threshold: %d%%\n", shortlistThreshold);
    fprintf(fp, "Rule: Final Score >= threshold is SHORTLISTED\n\n");

    for (int i = 0; i < candidateCount; i++) {
        Candidate *c = &candidates[i];
        fprintf(fp, "==============================================\n");
        fprintf(fp, "Candidate Name : %s\n", c->name);
        fprintf(fp, "Rank           : %d\n", c->rank);
        fprintf(fp, "Email          : %s\n", c->email);
        fprintf(fp, "Phone          : %s\n", c->phone);
        fprintf(fp, "Primary Domain : %s\n", c->primaryDomain);
        fprintf(fp, "Experience     : %d year(s)\n", c->experienceYears);
        fprintf(fp, "Degree         : %s\n", c->degree[0] ? c->degree : "N/A");
        fprintf(fp, "\nTop Skills:\n");
        for (int s = 0; s < c->topSkillCount; s++) fprintf(fp, "  - %s\n", c->topSkills[s]);
        fprintf(fp, "\nMatched Skills (%d):\n", c->matchedSkillCount);
        for (int s = 0; s < c->matchedSkillCount; s++) fprintf(fp, "  - %s\n", c->matchedSkills[s]);
        fprintf(fp, "\nMissing Skills (%d):\n", c->missingSkillCount);
        for (int s = 0; s < c->missingSkillCount; s++) fprintf(fp, "  - %s\n", c->missingSkills[s]);
        fprintf(fp, "\nMatch Percentage : %.1f%%\n", c->matchPercentage);
        fprintf(fp, "\nScore Breakdown:\n");
        fprintf(fp, "  Skill Match     : %d/50\n", c->skillScore);
        fprintf(fp, "  Experience      : %d/15\n", c->experienceScore);
        fprintf(fp, "  Education       : %d/10\n", c->educationScore);
        fprintf(fp, "  Projects        : %d/20\n", c->projectScore);
        fprintf(fp, "  Certifications  : %d/5\n", c->certScore);
        fprintf(fp, "  Final Score     : %d/100\n", c->finalScore);
        fprintf(fp, "Status           : %s\n", c->shortlisted ? "SHORTLISTED" : "Not shortlisted");
        fprintf(fp, "==============================================\n\n");
    }
    fclose(fp);
    printf("[report] GenerateTXT: wrote report for %d candidate(s) to '%s'\n", candidateCount, outFile);
    LogInfo("REPORT", "TXT report generated with score breakdown");
    return candidateCount;
}

int ExportReport(const char *outFile)
{
    EnsureDirectoryExists("output");
    EnsureDirectoryExists("output/reports");

    /* A combined, single-file export: summary line per candidate */
    FILE *fp = fopen(outFile, "w");
    if (!fp) {
        printf("[report] ExportReport failed: cannot open '%s'\n", outFile);
        return 0;
    }

    fprintf(fp, "HR RESUME SCREENING - FULL REPORT\n");
    fprintf(fp, "Automatic Shortlist Threshold: %d%%\n", shortlistThreshold);
    fprintf(fp, "Shortlisting Rule: Final Score >= %d%%\n", shortlistThreshold);
    fprintf(fp, "Total Candidates Processed: %d\n", candidateCount);
    int shortlistedCount = 0;
    for (int i = 0; i < candidateCount; i++) {
        if (candidates[i].shortlisted) shortlistedCount++;
    }
    fprintf(fp, "Total Shortlisted: %d\n\n", shortlistedCount);
    for (int i = 0; i < candidateCount; i++) {
        Candidate *c = &candidates[i];
        fprintf(fp, "#%d %-25s Score:%3d/100  Match:%.0f%%  %s\n",
                c->rank, c->name, c->finalScore, c->matchPercentage,
                c->shortlisted ? "[SHORTLISTED]" : "");
    }
    fclose(fp);
    printf("[report] ExportReport: summary written to '%s' (threshold %d%%)\n", outFile, shortlistThreshold);
    LogInfo("REPORT", "Full report generated with automatic shortlist threshold and status");
    return candidateCount;
}
