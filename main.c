/*
 * HR Resume Screening System using Top Word Frequency Analyzer
 * ---------------------------------------------------------------
 * Menu-driven console application.
 *
 *   Main Menu:   1. Login   2. Signup   3. Exit
 *
 * Per-resume processing pipeline (matches the documented process flow):
 *   HR Login -> Upload Job Requirement -> Upload Candidate Resumes ->
 *   Resume Text Extraction -> Text Cleaning -> Tokenization ->
 *   Stop Word Removal -> Top Word Frequency Analysis -> Skill
 *   Categorization -> Skill Matching -> Candidate Score Calculation ->
 *   Candidate Ranking -> Shortlisting -> Report Generation.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "auth.h"
#include "job_requirement.h"
#include "resume_upload.h"
#include "resume_processing.h"
#include "tokenizer.h"
#include "stopwords.h"
#include "analyzer.h"
#include "keyword_category.h"
#include "info_extractor.h"
#include "skill_match.h"
#include "scoring.h"
#include "ranking.h"
#include "shortlist.h"
#include "search.h"
#include "report.h"
#include "dashboard.h"
#include "utils.h"
#include "parallel_reader.h"
#include "logger.h"

/* ------------------------------------------------------------------ */
/* Small console input helpers                                        */
/* ------------------------------------------------------------------ */

static void ReadLine(char *buf, int size)
{
    if (fgets(buf, size, stdin) == NULL) {
        /* EOF or a read error (e.g. stdin piped/redirected and exhausted) -
         * fail safe with an empty string instead of leaving buf uninitialized. */
        buf[0] = '\0';
        return;
    }
    buf[strcspn(buf, "\r\n")] = '\0';
}

/* Reads a line and parses it as an integer. Returns -1 on blank/invalid
 * input instead of looping forever, so a stray Enter never hangs the menu. */
static int ReadIntChoice(void)
{
    char line[64];
    ReadLine(line, sizeof(line));
    int value;
    if (sscanf(line, "%d", &value) != 1) return -1;
    return value;
}

static void PressEnterToContinue(void)
{
    char tmp[16];
    printf("\nPress Enter to continue...");
    ReadLine(tmp, sizeof(tmp));
}

/* ------------------------------------------------------------------ */
/* Resume processing pipeline (per-resume steps, diagram-ordered)      */
/* ------------------------------------------------------------------ */

/*
 * Runs the analysis/scoring half of the pipeline after worker threads have
 * completed the independent I/O + text-preprocessing stage. The worker pool
 * already performed cleaning, normalization, tokenization and stop-word
 * removal. The remaining analysis stays single-threaded because the existing
 * analyzer/category/scoring modules use shared global state.
 */
static void ProcessResumeText(const char *filepath,
                               const char *rawText,
                               const char tokens[][MAX_TOKEN_LEN],
                               int tokenCount,
                               int rawTokenCount)
{
    if (currentCandidateIdx >= MAX_CANDIDATES) {
        printf("[main] Candidate limit reached, skipping remaining resumes\n");
        return;
    }

    Candidate *c = CURRENT;
    memset(c, 0, sizeof(Candidate));
    strncpy(c->filename, filepath, MAX_PATH_LEN - 1);
    c->filename[MAX_PATH_LEN - 1] = '\0';

    /* --- Resume information extraction --- */
    ExtractName(rawText, c->name);
    ExtractEmail(rawText, c->email);
    ExtractPhone(rawText, c->phone);
    ExtractSkills(rawText);
    ExtractProjects(rawText);
    ExtractCertifications(rawText);
    ExtractEducation(rawText);
    ExtractExperience(rawText);

    /* --- Text preprocessing was already performed by worker threads. --- */
    c->totalWords = rawTokenCount;
    printf("[main] Using %d preprocessed tokens for '%s'\n", tokenCount, filepath);

    /* --- Top Word Frequency Analysis --- */
    CountWordFrequency(tokens, tokenCount);
    FindTopKeywords(10);
    KeywordDensity("python", c->totalWords);
    ResumeStatistics();

    /* --- Skill Categorization --- */
    CategorizeKeywords();
    DomainDetection();

    /* --- Skill Matching --- */
    MatchSkills();
    MissingSkills();
    CalculateMatchPercentage(c->matchedSkillCount, jobSkillCount);

    /* --- Candidate Score Calculation --- */
    CalculateSkillScore();
    CalculateExperienceScore();
    CalculateEducationScore();
    CalculateProjectScore();
    CalculateCertificationScore();
    GenerateFinalScore();

    candidateCount++;
    currentCandidateIdx++;
}

static void ResetProcessingState(void)
{
    candidateCount = 0;
    currentCandidateIdx = 0;
    resumeFileCount = 0;
    jobSkillCount = 0;
}

/* ------------------------------------------------------------------ */
/* Job requirement submenu                                            */
/* ------------------------------------------------------------------ */

static void MenuRequirement(void)
{
    printf("\n--- Job Requirement ---\n");
    printf(" 1. Upload/Load Requirement File (.txt or .pdf)\n");
    printf(" 2. View Current Requirement\n");
    printf(" 3. Back\n");
    printf("Enter choice: ");
    int choice = ReadIntChoice();

    char path[MAX_PATH_LEN];

    switch (choice) {
        case 1:
            printf("\nEnter job requirement file path (.txt or .pdf)\n");
            printf("[Enter for default: data/job_requirement.pdf]: ");
            ReadLine(path, sizeof(path));
            if (strlen(path) == 0) strcpy(path, "data/job_requirement.pdf");
            UploadJobRequirement(path);
            break;
        case 2:
            if (jobSkillCount == 0) {
                printf("\nNo job requirement is loaded. Please use option 1 first.\n");
                LogWarning("JOB_REQUIREMENT", "View requested but no requirement is loaded");
                break;
            }
            printf("\n--- Current Job Requirement ---\n");
            printf("Required skills: %d\n", jobSkillCount);
            for (int i = 0; i < jobSkillCount; i++)
                printf("  %d. %s\n", i + 1, jobSkills[i]);
            LogInfo("JOB_REQUIREMENT", "Current job requirement viewed");
            break;
        case 3:
            return;
        default:
            printf("\nInvalid choice.\n");
            LogWarning("JOB_REQUIREMENT", "Invalid job requirement menu choice");
            break;
    }

}

/* ------------------------------------------------------------------ */
/* Resume upload submenu                                              */
/* ------------------------------------------------------------------ */

static void MenuUploadResumes(void)
{
    printf("\n--- Upload Resumes ---\n");
    printf(" 1. Bulk Upload (scan a folder)\n");
    printf(" 2. Upload a Single Resume\n");
    printf("Enter choice: ");
    int choice = ReadIntChoice();

    char path[MAX_PATH_LEN];
    switch (choice) {
        case 1:
            printf("\nEnter folder path containing resumes (.txt/.pdf)\n");
            printf("[Enter for default: data/resumes]: ");
            ReadLine(path, sizeof(path));
            if (strlen(path) == 0) strcpy(path, "data/resumes");
            BulkUpload(path);
            break;
        case 2:
            printf("\nEnter path to a single resume file (.txt or .pdf): ");
            ReadLine(path, sizeof(path));
            if (strlen(path) == 0) {
                printf("No path entered.\n");
                break;
            }
            UploadResume(path);
            break;
        default:
            printf("\nInvalid choice.\n");
    }
}

static void MenuProcessResumes(void)
{
    if (resumeFileCount == 0) {
        printf("\nNo resumes uploaded yet. Use 'Upload Resumes' first.\n");
        return;
    }
    if (jobSkillCount == 0) {
        printf("\nNo job requirement loaded yet. Use 'Job Requirement' first.\n");
        return;
    }

    candidateCount = 0;
    currentCandidateIdx = 0;

    /* --- Multithreaded resume-processing stage ---
     * Each worker independently performs file/PDF reading plus text
     * cleaning, normalization, tokenization and stop-word removal. The
     * later scoring pipeline remains sequential because its legacy modules
     * use shared global analysis state. */
    static ResumeReadResult readResults[MAX_RESUME_FILES];
    ReadResumesParallel(resumeFiles, resumeFileCount, readResults, DEFAULT_RESUME_WORKER_THREADS);

    /* --- Final analysis remains sequential to protect shared global state. --- */
    for (int i = 0; i < resumeFileCount; i++) {
        printf("\n---------------------------------------------------------\n");
        printf(" Processing: %s [%s]\n", readResults[i].filepath, readResults[i].statusMsg);
        printf("---------------------------------------------------------\n");

        if (!readResults[i].text) {
            printf("[main] Skipping '%s' (unreadable)\n", readResults[i].filepath);
            continue;
        }

        if (!readResults[i].tokens) {
            printf("[main] Skipping '%s' (preprocessing failed)\n", readResults[i].filepath);
            free(readResults[i].text);
            readResults[i].text = NULL;
            free(readResults[i].workText);
            readResults[i].workText = NULL;
            continue;
        }

        ProcessResumeText(readResults[i].filepath,
                          readResults[i].text,
                          (const char (*)[MAX_TOKEN_LEN])readResults[i].tokens,
                          readResults[i].tokenCount,
                          readResults[i].rawTokenCount);

        free(readResults[i].text);
        readResults[i].text = NULL;
        free(readResults[i].workText);
        readResults[i].workText = NULL;
        free(readResults[i].tokens);
        readResults[i].tokens = NULL;
    }

    /* --- Candidate Ranking and automatic score-based shortlisting --- */
    if (candidateCount > 0) {
        RankCandidates();

        printf("\nAutomatic Shortlisting\n");
        printf("Enter minimum score threshold (0-100) [default %d]: ", GetShortlistThreshold());
        char thresholdLine[32];
        ReadLine(thresholdLine, sizeof(thresholdLine));

        int threshold = GetShortlistThreshold();
        if (strlen(thresholdLine) > 0) {
            if (sscanf(thresholdLine, "%d", &threshold) != 1 || threshold < 0 || threshold > 100) {
                printf("Invalid threshold. Keeping previous threshold of %d%%.\n", threshold);
                threshold = GetShortlistThreshold();
                LogWarning("SHORTLIST", "Invalid HR threshold entered; previous threshold retained");
            }
        }

        ShortlistByScoreThreshold(threshold);
        GenerateShortlist();

        /* Generate reports immediately so HR receives the shortlist and
         * complete screening report in the same workflow. */
        int csvRows = GenerateCSV("output/reports/Candidate_Report.csv");
        int txtRows = GenerateTXT("output/reports/Candidate_Report.txt");
        int summaryRows = ExportReport("output/reports/Full_Report.txt");
        if (csvRows == candidateCount && txtRows == candidateCount && summaryRows == candidateCount)
            printf("[main] Automatic shortlist and reports generated successfully.\n");
        else
            LogError("REPORT", "One or more automatic reports could not be generated");
    }
    printf("\n[main] Processed %d resume(s).\n", candidateCount);
}

static void MenuViewRanking(void)
{
    if (candidateCount == 0) {
        printf("\nNo candidates processed yet. Use 'Process Uploaded Resumes' first.\n");
        return;
    }
    RankCandidates();
}

/* --- Score-threshold shortlisting --- */
static void MenuShortlist(void)
{
    if (candidateCount == 0) {
        printf("\nNo candidates processed yet.\n");
        return;
    }

    printf("\nCurrent automatic shortlist threshold: %d%%\n", GetShortlistThreshold());
    printf("Enter new minimum score threshold (0-100) [Enter to keep %d]: ", GetShortlistThreshold());
    char line[32];
    ReadLine(line, sizeof(line));

    int threshold = GetShortlistThreshold();
    if (strlen(line) > 0) {
        if (sscanf(line, "%d", &threshold) != 1 || threshold < 0 || threshold > 100) {
            printf("Invalid threshold. Keeping %d%%.\n", threshold);
            threshold = GetShortlistThreshold();
        }
    }

    RankCandidates();
    ShortlistByScoreThreshold(threshold);
    GenerateShortlist();
    GenerateCSV("output/reports/Candidate_Report.csv");
    GenerateTXT("output/reports/Candidate_Report.txt");
    ExportReport("output/reports/Full_Report.txt");
    printf("[main] Shortlist and reports regenerated using threshold %d%%.\n", GetShortlistThreshold());
}

static void MenuSearch(void)
{
    if (candidateCount == 0) {
        printf("\nNo candidates processed yet.\n");
        return;
    }

    printf("\n--- Search Candidates ---\n");
    printf(" 1. Search by Name\n");
    printf(" 2. Search by Skill\n");
    printf(" 3. Search by Minimum Experience\n");
    printf(" 4. Search by Email\n");
    printf(" 5. Filter by Degree\n");
    printf("Enter choice: ");
    int choice = ReadIntChoice();

    char buf[100];
    switch (choice) {
        case 1:
            printf("Name: ");
            ReadLine(buf, sizeof(buf));
            SearchByName(buf);
            break;
        case 2:
            printf("Skill: ");
            ReadLine(buf, sizeof(buf));
            SearchBySkill(buf);
            break;
        case 3: {
            printf("Minimum years of experience: ");
            int years = ReadIntChoice();
            if (years < 0) years = 0;
            SearchByExperience(years);
            break;
        }
        case 4:
            printf("Email: ");
            ReadLine(buf, sizeof(buf));
            SearchByEmail(buf);
            break;
        case 5:
            printf("Degree keyword (e.g. B.Tech, BCA): ");
            ReadLine(buf, sizeof(buf));
            FilterByDegree(buf);
            break;
        default:
            printf("Invalid choice.\n");
    }
}

/* --- Report Generation --- */
static void MenuReports(void)
{
    if (candidateCount == 0) {
        printf("\nNo candidates processed yet.\n");
        return;
    }
    int csvRows = GenerateCSV("output/reports/Candidate_Report.csv");
    int txtRows = GenerateTXT("output/reports/Candidate_Report.txt");
    int summaryRows = ExportReport("output/reports/Full_Report.txt");

    if (csvRows > 0 && txtRows > 0 && summaryRows > 0)
        printf("\n[main] All reports written successfully to output/reports/\n");
    else
        printf("\n[main] One or more reports could not be written - see messages above.\n");
}

static void MenuDashboard(void)
{
    if (candidateCount == 0) {
        printf("\nNo candidates processed yet.\n");
        return;
    }
    DisplayStatistics();
    DisplayTopSkills();
    DisplaySummary();
}

static void MenuChangePassword(void)
{
    char oldPw[100], newPw[100];
    printf("\nCurrent password: ");
    ReadLine(oldPw, sizeof(oldPw));
    printf("New password: ");
    ReadLine(newPw, sizeof(newPw));
    ChangePassword(loggedInUser, oldPw, newPw);
}

/* ------------------------------------------------------------------ */
/* HR menu loop (shown after a successful login)                      */
/* ------------------------------------------------------------------ */

static void HRMenuLoop(void)
{
    /* The HR menu is accessible only after a successful authentication. */
    if (!isLoggedIn) {
        printf("\n[auth] Access denied: please login first.\n");
        LogWarning("AUTH", "Attempted to open HR main menu without an active session");
        return;
    }

    LogInfo("MENU", "HR main menu opened after successful login");

    for (;;) {
        printf("\n=========================================================\n");
        printf(" HR Resume Screening System - Main Menu   (User: %s)\n", loggedInUser);
        printf("=========================================================\n");
        printf("  1. Job Requirement (upload/view)\n");
        printf("  2. Upload Resumes (bulk or single)\n");
        printf("  3. Process Uploaded Resumes\n");
        printf("  4. View Candidate Ranking\n");
        printf("  5. Automatic Shortlist by Score Threshold\n");
        printf("  6. Search Candidates\n");
        printf("  7. Generate Reports\n");
        printf("  8. View Analytics Dashboard\n");
        printf("  9. Change Password\n");
        printf(" 10. Logout\n");
        printf("  0. Exit Program\n");
        printf("---------------------------------------------------------\n");
        printf("Enter choice: ");

        int choice = ReadIntChoice();
        switch (choice) {
            case 1: MenuRequirement(); break;
            case 2: MenuUploadResumes(); break;
            case 3: MenuProcessResumes(); break;
            case 4: MenuViewRanking(); break;
            case 5: MenuShortlist(); break;
            case 6: MenuSearch(); break;
            case 7: MenuReports(); break;
            case 8: MenuDashboard(); break;
            case 9: MenuChangePassword(); break;
            case 10:
                Logout();
                ResetProcessingState();
                return; /* back to the Login/Signup/Exit main menu */
            case 0:
                Logout();
                printf("\nGoodbye!\n");
                exit(0);
            default:
                printf("\nInvalid choice, please try again.\n");
                LogWarning("MENU", "Invalid main menu choice");
                continue;
        }
        PressEnterToContinue();
    }
}

/* ------------------------------------------------------------------ */
/* Main menu: Login / Signup / Exit                                   */
/* ------------------------------------------------------------------ */

static void DoLogin(void)
{
    char username[100], password[200];
    printf("\n--- Login ---\n");
    printf("Email address: ");
    ReadLine(username, sizeof(username));
    printf("Password: ");
    ReadLine(password, sizeof(password));

    /* Do not keep credentials in memory longer than necessary. */
    int loginOk = Login(username, password);
    memset(password, 0, sizeof(password));

    if (loginOk) {
        /* Login() sets isLoggedIn. Only then is the HR main menu opened. */
        HRMenuLoop();
    } else {
        PressEnterToContinue();
    }
}

static void DoSignup(void)
{
    char username[100], password[200];
    printf("\n--- Signup ---\n");
    printf("Email address (unique): ");
    ReadLine(username, sizeof(username));
    printf("Choose a password (min 8 chars, letters + digits): ");
    ReadLine(password, sizeof(password));

    int signupOk = Signup(username, password);
    memset(password, 0, sizeof(password));

    if (signupOk) {
        /* Required flow: successful signup -> login screen.
         * The user must authenticate with the newly created account;
         * signup does not automatically create a logged-in session. */
        LogInfo("AUTH", "Signup completed; redirecting user to login");
        printf("\nSignup successful. Please login with your new account.\n");
        DoLogin();
    } else {
        /* Failed signup stays outside the authenticated menu. */
        PressEnterToContinue();
    }
}

int main(void)
{
    /* Create the log directory BEFORE opening the logger. This guarantees
     * that every normal application run creates/updates the timestamped
     * log file at output/logs/hr_resume_screening.log. */
    EnsureDirectoryExists("output");
    EnsureDirectoryExists("output/reports");
    EnsureDirectoryExists("output/shortlisted");
    EnsureDirectoryExists("output/analysis");
    EnsureDirectoryExists("output/logs");
    EnsureDirectoryExists("output/extracted_text");

    InitLogger();
    atexit(CloseLogger);
    LogInfo("SYSTEM", "Application started");
    LogInfo("AUTH", "Application ready; awaiting signup or login");

    /* Reference data (skill list, stopwords, categories) is not sensitive,
     * so it's loaded once at startup regardless of login state. */
    LoadTechnicalSkills("data/skills.txt");
    LoadStopWords("data/stopwords.txt");
    LoadCategories("data/categories.txt");

    for (;;) {
        printf("\n=========================================================\n");
        printf(" HR Resume Screening System - Top Word Frequency Analyzer\n");
        printf("=========================================================\n");
        printf("  1. Login\n");
        printf("  2. Signup\n");
        printf("  3. Exit\n");
        printf("---------------------------------------------------------\n");
        printf("Enter choice: ");

        int choice = ReadIntChoice();
        switch (choice) {
            case 1: DoLogin(); break;
            case 2: DoSignup(); break;
            case 3:
                printf("\nGoodbye!\n");
                LogInfo("SYSTEM", "Application exit requested");
                return 0;
            default:
                printf("\nInvalid choice, please try again.\n");
                LogWarning("MENU", "Invalid main menu choice");
        }
    }
}
